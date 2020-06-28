#include "writer/verilog/state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_attr.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/ext_task.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"

static const char I[] = "        ";

namespace iroha {
namespace writer {
namespace verilog {

State::State(IState *i_state, Table *table, Names *names)
  : i_state_(i_state), table_(table), names_(names),
    transition_insn_(nullptr) {
  is_compound_cycle_ = ResourceAttr::NumMultiCycleInsn(i_state) > 0;
  transition_insn_ = DesignUtil::FindTransitionInsn(i_state);
}

State::~State() {
}

void State::Build() {
  ostream &ws = table_->InsnWireValueSectionStream();
  for (auto *insn : i_state_->insns_) {
    auto *ires = insn->GetResource();
    Resource *res = table_->GetResource(ires);
    if (res != nullptr) {
      res->BuildInsn(insn, this);
    }
    if (!resource::IsSet(*ires->GetClass())) {
      CopyResults(insn, true, ws);
    }
  }
}

void State::Write(ostream &os) {
  os << I << table_->StateName(i_state_->GetId()) << ": begin\n";
  WriteStateBody(os);
  WriteTransition(os);
  os << I << "end\n";
}

void State::WriteStateBody(ostream &os) {
  os << StateBodySectionContents();
  if (is_compound_cycle_) {
    os << I << "  if (" << MultiCycleTransitionCond() << ") begin\n"
       << I << "    // 1 cycle insns\n";
  }
  os << StateTransitionSectionContents();
  for (auto *insn : i_state_->insns_) {
    auto *res = insn->GetResource();
    if (!resource::IsSet(*res->GetClass())) {
      CopyResults(insn, false, os);
    }
  }
  if (is_compound_cycle_) {
    os << I << "  end\n";
  }
}

const IState *State::GetIState() const {
  return i_state_;
}

void State::CopyResults(const IInsn *insn, bool to_wire, ostream &os) {
  int nth = 0;
  for (auto *oreg : insn->outputs_) {
    if (oreg->IsStateLocal() && to_wire) {
      os << "  assign ";
    } else if (!oreg->IsStateLocal() && !to_wire) {
      os << I << "  ";
    } else {
      return;
    }
    os << InsnWriter::RegisterValue(*oreg, names_);
    if (to_wire) {
      os << " = ";
    } else {
      os << " <= ";
    }
    os << InsnWriter::InsnOutputWireName(*insn, nth) << ";\n";
    ++nth;
  }
}

void State::WriteTransitionBody(ostream &os) {
  const string &sv = table_->StateVariable();
  if (DesignUtil::IsTerminalState(i_state_) &&
      table_->IsTaskOrExtTask()) {
    os << I << "  " << sv << " <= "
       << table_->StateName(Task::kTaskEntryStateId)
       << ";\n";
    return;
  }
  if (transition_insn_ == nullptr ||
      transition_insn_->target_states_.size() == 0) {
    return;
  }
  if (transition_insn_->target_states_.size() == 1) {
    os << I << "  " << sv << " <= "
       << table_->StateName(transition_insn_->target_states_[0]->GetId())
       << ";\n";
    return;
  }
  IRegister *cond = transition_insn_->inputs_[0];
  os << I << "  if (" << InsnWriter::RegisterValue(*cond, names_) << ") begin\n"
     << I << "    " << sv << " <= "
     << table_->StateName(transition_insn_->target_states_[1]->GetId())
     << ";\n"
     << I << "  end else begin\n"
     << I << "    " << sv << " <= "
     << table_->StateName(transition_insn_->target_states_[0]->GetId())
     << ";\n"
     << I << "  end\n";
}

void State::WriteTransition(ostream &os) {
  if (is_compound_cycle_) {
    ClearMultiCycleState(os);
    os << I << "  if (" << MultiCycleTransitionCond()
       << ") begin\n";
  }
  WriteTransitionBody(os);
  if (is_compound_cycle_) {
    os << I << "  end\n";
  }
}

void State::ClearMultiCycleState(ostream &os) {
  os << I << "  if ("
     << MultiCycleTransitionCond() << ") begin\n"
     << I << "    // clears sub states\n";
  for (IInsn *insn : i_state_->insns_) {
    if (ResourceAttr::IsMultiCycleInsn(insn)) {
      string st = InsnWriter::MultiCycleStateName(*insn->GetResource());
      os << I << "    " << st << " <= " << "0;\n";
    }
  }
  os << I << "  end\n";
}

string State::MultiCycleTransitionCond() {
  vector<string> sts;
  for (IInsn *insn : i_state_->insns_) {
    if (!ResourceAttr::IsMultiCycleInsn(insn)) {
      continue;
    }
    IResource *res = insn->GetResource();
    string st = InsnWriter::MultiCycleStateName(*res);
    st = "(" + st + " == 3)";
    sts.push_back(st);
  }
  return Util::Join(sts, " && ");
}

void State::WriteTaskEntry(Table *tab, ostream &os) {
  os << I << tab->StateName(Task::kTaskEntryStateId)
     << ": begin\n";
  string s;
  if (Task::IsTask(*tab)) {
    s = Task::TaskEnableWire(*tab->GetITable());
  } else {
    s = ExtTask::TaskReadyPin(*tab);
  }
  os << I << "  if (" << s << ") begin\n"
     << I << "    " << tab->StateVariable() << " <= "
     << tab->InitialStateName() << ";\n"
     << tab->TaskEntrySectionContents()
     << I << "  end\n"
     << I << "end\n";
}

bool State::IsCompoundCycle() const {
  return is_compound_cycle_;
}

Names *State::GetNames() const {
  return names_;
}

ostream &State::StateBodySectionStream() const {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  return tmpl->GetStream(StateBodySectionName());
}

string State::StateBodySectionContents() const {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  return tmpl->GetContents(StateBodySectionName());
}

ostream &State::StateTransitionSectionStream() const {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  return tmpl->GetStream(StateTransitionSectionName());
}

string State::StateTransitionSectionContents() const {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  return tmpl->GetContents(StateTransitionSectionName());
}

string State::StateBodySectionName() const {
  return kStateBodySection +
    Util::Itoa(table_->GetITable()->GetId()) +
    ":" + Util::Itoa(i_state_->GetId());
}

string State::StateTransitionSectionName() const {
  return kStateTransitionSection +
    Util::Itoa(table_->GetITable()->GetId()) +
    ":" + Util::Itoa(i_state_->GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
