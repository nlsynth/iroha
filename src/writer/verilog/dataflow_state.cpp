#include "writer/verilog/dataflow_state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "writer/verilog/insn_writer.h"

namespace iroha {
namespace writer {
namespace verilog {

DataFlowState::DataFlowState(IState *state, Table *table, Names *names)
  : State(state, table, names) {
}

DataFlowState::~DataFlowState() {
}

void DataFlowState::BuildIncomingTransitions(const vector<DataFlowStateTransition> &trs) {
  if (trs.size() == 0) {
    return;
  }
  vector<string> conds;
  for (auto &tr : trs) {
    const IState *prev_st = tr.from->GetIState();
    string s;
    if (prev_st->GetTable()->GetInitialState() == prev_st) {
      // Comes from initial state.
      IInsn *insn = DesignUtil::FindDataFlowInInsn(prev_st->GetTable());
      s = InsnWriter::RegisterValue(*insn->inputs_[0], names_);
    } else {
      s = StateVariable(prev_st);
    }
    if (!tr.cond.empty()) {
      s = "(" + s + " && " + tr.cond + ")";
    }
    conds.push_back(s);
  }
  string c = Util::Join(conds, " || ");
  incoming_transitions_ =
    "      " + StateVariable(i_state_) + " <= " + c + ";\n";
}

void DataFlowState::Write(ostream &os) {
  string s;
  if (i_state_->GetTable()->GetInitialState() == i_state_) {
    IInsn *insn = DesignUtil::FindDataFlowInInsn(i_state_->GetTable());
    s = InsnWriter::RegisterValue(*insn->inputs_[0], names_);
  } else {
    s = StateVariable(i_state_);
  }
  os << incoming_transitions_;
  os << "      if (" << s << ") begin\n";
  WriteStateBody(os);
  os << "      end\n";
}

string DataFlowState::StateVariable(const IState *st) {
  return "st_" + Util::Itoa(st->GetTable()->GetId()) + "_" +
    Util::Itoa(st->GetId());
}

vector<DataFlowStateTransition> DataFlowState::GetTransitions() {
  vector<DataFlowStateTransition> trs;
  if (transition_insn_ == nullptr) {
    return trs;
  }
  for (IState *ts : transition_insn_->target_states_) {
    DataFlowStateTransition tr;
    tr.to = nullptr;
    tr.to_raw = ts;
    tr.from = this;
    if (transition_insn_->target_states_.size() == 2) {
      IRegister *cond = transition_insn_->inputs_[0];
      if (ts == transition_insn_->target_states_[0]) {
	tr.cond = "!" + InsnWriter::RegisterValue(*cond, names_);
      } else {
	tr.cond = InsnWriter::RegisterValue(*cond, names_);
      }
    }
    trs.push_back(tr);
  }
  return trs;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
