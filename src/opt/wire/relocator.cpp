#include "opt/wire/relocator.h"

#include "design/design_tool.h"
#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "opt/bb_set.h"
#include "opt/wire/data_path.h"
#include "opt/wire/path_node.h"

namespace iroha {
namespace opt {
namespace wire {

Relocator::Relocator(DataPathSet *data_path_set)
  : data_path_set_(data_path_set) {
  ITable *tab = data_path_set->GetBBSet()->GetTable();
  assign_ = DesignTool::GetOneResource(tab, resource::kSet);
}

void Relocator::Relocate() {
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
    int st_index = n->GetFinalStIndex();
    states[st_index]->insns_.push_back(n->GetInsn());
  }
  // Wires.
  RewirePaths(dp, &states);
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
  BB *bb = dp->GetBB();
  auto &nodes = dp->GetNodes();
  for (auto &p : nodes) {
    PathNode *src_node = p.second;
    IState *src_st = states->at(src_node->GetFinalStIndex());
    for (auto &q : src_node->sink_edges_) {
      PathEdge *edge = q.second;
      IRegister *reg = edge->GetSourceReg();
      if (src_node->GetFinalStIndex() == edge->GetSinkNode()->GetFinalStIndex()) {
	// Source and Sink are in same state.
	if (reg->IsStateLocal()) {
	  // Does nothing. Just use the wire.
	} else {
	  // Replace the output to a wire.
	  // orig-insn: wire <- 
	  // new-assign-insn: orig-output <- wire
	  AddIntermediateWireAndInsn(edge, src_st);
	}
      } else {
	// Sink uses the value in a state later.
	if (reg->IsStateLocal()) {
	  // Add assign insn 
	  AddIntermediateRegAndInsn(edge, src_st);
	} else {
	  // Does nothing.
	}
      }
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
  IRegister *reg = new IRegister(table, name);
  reg->SetStateLocal(state_local);
  reg->value_type_ = orig_reg->value_type_;
  table->registers_.push_back(reg);
  return reg;
}

void Relocator::AddIntermediateWireAndInsn(PathEdge *edge, IState *st) {
  IRegister *orig_out = edge->GetSourceReg();
  IRegister *wire = AllocIntermediateReg(edge->GetSourceNode()->GetInsn(),
					 true, edge->GetSourceRegIndex());
  edge->SetSourceReg(wire);
  IInsn *assign_insn = new IInsn(assign_);
  assign_insn->inputs_.push_back(wire);
  assign_insn->outputs_.push_back(orig_out);
  st->insns_.push_back(assign_insn);
}

void Relocator::AddIntermediateRegAndInsn(PathEdge *edge, IState *st) {
  IRegister *orig_out = edge->GetSourceReg();
  IRegister *reg = AllocIntermediateReg(edge->GetSourceNode()->GetInsn(),
					false, edge->GetSourceRegIndex());
  IInsn *assign_insn = new IInsn(assign_);
  assign_insn->inputs_.push_back(reg);
  assign_insn->outputs_.push_back(orig_out);
  st->insns_.push_back(assign_insn);
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
