#include "writer/verilog/dataflow_state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "writer/verilog/insn_writer.h"

namespace iroha {
namespace writer {
namespace verilog {

DataFlowState::DataFlowState(IState *state, Table *table)
  : State(state, table) {
}

DataFlowState::~DataFlowState() {
}

void DataFlowState::Write(ostream &os) {
  string s;
  if (i_state_->GetTable()->GetInitialState() == i_state_) {
    IInsn *insn = DesignUtil::FindDataFlowInInsn(i_state_->GetTable());
    s = InsnWriter::RegisterName(*insn->inputs_[0]);
  } else {
    s = StateVariable(i_state_);
  }
  os << "      if (" << s << ") begin\n";
  if (transition_insn_ != nullptr) {
    if (transition_insn_->target_states_.size() == 1) {
      IState *ns = transition_insn_->target_states_[0];
      os << "        " << StateVariable(ns) << " <= 1;\n";
    } else {
      IRegister *cond = transition_insn_->inputs_[0];
      IState *ns0 = transition_insn_->target_states_[0];
      os << "        " << StateVariable(ns0)
	 << " <= !" << InsnWriter::RegisterName(*cond) << "\n";
      IState *ns1 = transition_insn_->target_states_[1];
      os << "        " << StateVariable(ns1)
	 << " <= " << InsnWriter::RegisterName(*cond) << "\n";
    }
  }
  WriteStateBody(os);
  os << "      end else begin\n";
  if (transition_insn_ != nullptr) {
    for (IState *ts : transition_insn_->target_states_) {
      os << "        " << StateVariable(ts) << " <= 0;\n";
    }
  }
  os << "      end\n";
}

string DataFlowState::StateVariable(const IState *st) {
  return "st_" + Util::Itoa(st->GetTable()->GetId()) + "_" +
    Util::Itoa(st->GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
