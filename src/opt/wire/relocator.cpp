#include "opt/wire/relocator.h"

#include "design/design_tool.h"
#include "iroha/i_design.h"
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
  // Clears and places insns.
  // TODO: Handle transition insn.
  for (IState *st : bb->states_) {
    st->insns_.clear();
  }
  for (auto &p : nodes) {
    PathNode *n = p.second;
    bb->states_[n->GetFinalStIndex()]->insns_.push_back(n->GetInsn());
  }
  for (auto &p : nodes) {
    PathNode *src_node = p.second;
    IState *src_st = bb->states_[src_node->GetFinalStIndex()];
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
