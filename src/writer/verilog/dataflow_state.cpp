#include "writer/verilog/dataflow_state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "writer/verilog/dataflow_table.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/shared_reg.h"

namespace iroha {
namespace writer {
namespace verilog {

DataFlowState::DataFlowState(IState *state, DataFlowTable *table, Names *names)
  : State(state, table, names), df_table_(table) {
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
      s = StartCondition(insn);
    } else {
      s = StateVariable(prev_st);
    }
    if (!tr.cond.empty()) {
      s = "(" + s + " && " + tr.cond + ")";
    }
    conds.push_back(s);
  }
  string c = Util::Join(conds, " || ");
  if (df_table_->CanBlock()) {
    //    c = "(" + c + ") && !" + df_table_->BlockingCondition();
    c = df_table_->BlockingCondition() + " ? 0 : " + c;
  }
  incoming_transitions_ =
    "      " + StateVariable(i_state_) + " <= " + c + ";\n";
}

void DataFlowState::Write(ostream &os) {
  string s;
  if (i_state_->GetTable()->GetInitialState() == i_state_) {
    IInsn *insn = DesignUtil::FindDataFlowInInsn(i_state_->GetTable());
    s = StartCondition(insn);
    if (df_table_->CanBlock()) {
      s += " && !" + df_table_->BlockingCondition();
    }
  } else {
    s = StateVariable(i_state_);
  }
  os << "      // State: " << i_state_->GetId() << "\n";
  os << incoming_transitions_;
  if (df_table_->CanBlock()) {
    os << "      " << StateWaitVariable(i_state_)
       << " <= " << StateVariable(i_state_) << " && "
       << df_table_->BlockingCondition() << ";\n";
  }
  os << "      if (" << s << ") begin\n";
  WriteStateBody(os);
  if (is_compound_cycle_) {
    ClearMultiCycleState(os);
  }
  os << "      end\n";
}

string DataFlowState::StateVariable(const IState *st) {
  return "st_" + Util::Itoa(st->GetTable()->GetId()) + "_" +
    Util::Itoa(st->GetId());
}

string DataFlowState::StateWaitVariable(const IState *st) {
  return StateVariable(st) + "_w";
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

string DataFlowState::StartCondition(IInsn *insn) {
  IResource *res = insn->GetResource();
  IResource *p = res->GetParentResource();
  if (p != nullptr &&
      resource::IsSharedReg(*(p->GetClass()))) {
    return SharedReg::RegNotifierName(*p);
  } else {
    return InsnWriter::RegisterValue(*insn->inputs_[0], names_);
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
