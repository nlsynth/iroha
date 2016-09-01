#include "writer/verilog/dataflow_state.h"

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
  os << "      if (" << StateVariable(i_state_) << ") begin\n";
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
