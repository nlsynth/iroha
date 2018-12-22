#include "opt/wire/relocator.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/wire/data_path.h"
#include "opt/wire/path_node.h"
#include "opt/wire/resource_entry.h"
#include "opt/wire/resource_replica_assigner.h"
#include "opt/wire/virtual_resource.h"

namespace iroha {
namespace opt {
namespace wire {

Relocator::Relocator(DataPathSet *data_path_set)
  : data_path_set_(data_path_set) {
  ITable *tab = data_path_set->GetBBSet()->GetTable();
  assign_ = DesignTool::GetOneResource(tab, resource::kSet);
}

void Relocator::Relocate() {
  // Actually allocates replica resources.
  data_path_set_->GetVirtualResourceSet()->PrepareReplicas();
  // Assigns replica indexes.
  ResourceReplicaAssigner assigner(data_path_set_);
  assigner.Perform();
  // Move insns and may update IResource.
  auto &paths = data_path_set_->GetPaths();
  for (auto &p : paths) {
    RelocateInsnsForDataPath(p.second);
  }
}

void Relocator::RelocateInsnsForDataPath(BBDataPath *dp) {
  BB *bb = dp->GetBB();
  auto &nodes = dp->GetNodes();
  vector<IState *> states;
  // Clears and places insns.
  for (IState *st : bb->states_) {
    st->insns_.clear();
  }
  // Transition insns.
  RelocateTransitionInsns(dp, &states);
  // Non transition insns.
  for (auto &p : nodes) {
    PathNode *n = p.second;
    if (n->IsTransition()) {
      continue;
    }
    RelocateInsn(n, &states);
  }
  // Wires.
  RewirePaths(dp, &states);
}

void Relocator::RelocateInsn(PathNode *node, vector<IState *> *states) {
  int st_index = node->GetFinalStIndex();
  IInsn *insn = node->GetInsn();
  states->at(st_index)->insns_.push_back(insn);
  VirtualResource *vres = node->GetVirtualResource();
  int ridx = vres->GetReplicaIndex();
  if (ridx > 0) {
    ResourceEntry *rent = vres->GetResourceEntry();
    insn->SetResource(rent->GetNthReplica(ridx));
  }
}

void Relocator::RelocateTransitionInsns(BBDataPath *dp, vector<IState *> *states) {
  BB *bb = dp->GetBB();
  auto &nodes = dp->GetNodes();
  int max_st = 0;
  for (auto &p : nodes) {
    PathNode *n = p.second;
    int st_index = n->GetFinalStIndex();
    if (max_st < st_index) {
      max_st = st_index;
    }
  }
  // Appends states if necessary.
  *states = bb->states_;
  ITable *tab = states->at(0)->GetTable();
  while (max_st >= states->size()) {
    states->push_back(new IState(tab));
  }
  // Copies the original transition insns.
  for (auto &p : nodes) {
    PathNode *n = p.second;
    if (n->IsTransition()) {
      int st_index = n->GetFinalStIndex();
      states->at(st_index)->insns_.push_back(n->GetInsn());
    }
  }
  // Adjust transition targets and conditions.
  if (bb->states_.size() < states->size()) {
    //
    // 0th state shouldn't be touched becaused other BBs may reference it.
    // 0 1 ... bb->size()
    // \|/
    // 0 1 ... bb->size() bb->size()+1 ...   states->size()
    //                    |--- new states -----------------|
    //                                     | copy condition|
    IResource *tr = DesignUtil::FindTransitionResource(tab);
    for (int i = bb->states_.size(); i < states->size(); ++i) {
      IInsn *insn = new IInsn(tr);
      states->at(i)->insns_.push_back(insn);
    }
    IInsn *orig_last_insn = states->at(bb->states_.size() - 1)->insns_[0];
    IInsn *new_last_insn = states->at(states->size() - 1)->insns_[0];
    new_last_insn->inputs_ = orig_last_insn->inputs_;
    new_last_insn->target_states_ = orig_last_insn->target_states_;
    orig_last_insn->inputs_.clear();
    orig_last_insn->target_states_.clear();
    for (int i = bb->states_.size() - 1; i < states->size() - 1; ++i) {
      IInsn *cur_tr = states->at(i)->insns_[0];
      IState *next = states->at(i + 1);
      cur_tr->target_states_.push_back(next);
    }
  }
}

void Relocator::RewirePaths(BBDataPath *dp, vector<IState *> *states) {
  auto &nodes = dp->GetNodes();
  for (auto &p : nodes) {
    PathNode *src_node = p.second;
    RewirePathsNode(src_node, states);
  }
}

void Relocator::RewirePathsNode(PathNode *src_node, vector<IState *> *states) {
  IState *src_st = states->at(src_node->GetFinalStIndex());
  IInsn *src_insn = src_node->GetInsn();
  vector<IRegister *> output_wires(src_insn->outputs_.size());
  vector<IRegister *> output_regs(src_insn->outputs_.size());
  for (auto &q : src_node->sink_edges_) {
    PathEdge *edge = q.second;
    if (!edge->IsWtoR()) {
      continue;
    }
    int idx = edge->GetSourceRegIndex();
    IRegister *reg = edge->GetSourceReg();
    if (src_node->GetFinalStIndex() == edge->GetSinkNode()->GetFinalStIndex()) {
      // Source and Sink are in same state.
      if (reg->IsStateLocal()) {
	// Does nothing. Just use the wire.
      } else {
	// Replace the output reg to a wire.
	// orig-insn: wire <-
	// new-assign-insn: orig-output <- wire
	IRegister *wire = output_wires[idx];
	if (wire == nullptr) {
	  wire = AddIntermediateWireAndInsn(edge, src_st);
	  output_wires[idx] = wire;
	}
	RewriteSinkInput(edge, reg, wire);
      }
    } else {
      // Sink uses the value in a state later.
      if (reg->IsStateLocal()) {
	// Add assign insn
	IRegister *new_reg = output_regs[idx];
	if (new_reg == nullptr) {
	  new_reg = AddIntermediateRegAndInsn(edge, src_st);
	  output_regs[idx] = new_reg;
	}
	RewriteSinkInput(edge, reg, new_reg);
      } else {
	// Does nothing.
      }
    }
  }
  for (int i = 0; i < output_wires.size(); ++i) {
    IRegister *w = output_wires[i];
    if (w != nullptr) {
      src_insn->outputs_[i] = w;
    }
  }
}

void Relocator::RewriteSinkInput(PathEdge *edge, IRegister *src, IRegister *dst) {
  PathNode *sink_node = edge->GetSinkNode();
  IInsn *sink_insn = sink_node->GetInsn();
  for (int i = 0; i < sink_insn->inputs_.size(); ++i) {
    if (sink_insn->inputs_[i] == src) {
      sink_insn->inputs_[i] = dst;
    }
  }
}

IRegister* Relocator::AllocIntermediateReg(IInsn *insn, bool state_local,
					   int oindex) {
  ITable *table = insn->GetResource()->GetTable();
  IRegister *orig_reg = insn->outputs_[oindex];
  string name = orig_reg->GetName();
  if (state_local) {
    name += "_w";
  } else {
    name += "_r";
  }
  name += Util::Itoa(oindex);
  int iid = insn->GetId();
  if (iid > -1) {
    name += "i" + Util::Itoa(iid);
  }
  IRegister *reg = new IRegister(table, name);
  reg->SetStateLocal(state_local);
  reg->value_type_ = orig_reg->value_type_;
  table->registers_.push_back(reg);
  return reg;
}

IRegister *Relocator::AddIntermediateWireAndInsn(PathEdge *edge, IState *st) {
  IRegister *orig_out = edge->GetSourceReg();
  IRegister *wire = AllocIntermediateReg(edge->GetSourceNode()->GetInsn(),
					 true, edge->GetSourceRegIndex());
  IInsn *assign_insn = new IInsn(assign_);
  assign_insn->inputs_.push_back(wire);
  assign_insn->outputs_.push_back(orig_out);
  st->insns_.push_back(assign_insn);
  return wire;
}

IRegister *Relocator::AddIntermediateRegAndInsn(PathEdge *edge, IState *st) {
  IRegister *orig_out = edge->GetSourceReg();
  IRegister *reg = AllocIntermediateReg(edge->GetSourceNode()->GetInsn(),
					false, edge->GetSourceRegIndex());
  IInsn *assign_insn = new IInsn(assign_);
  assign_insn->inputs_.push_back(reg);
  assign_insn->outputs_.push_back(orig_out);
  st->insns_.push_back(assign_insn);
  return reg;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
