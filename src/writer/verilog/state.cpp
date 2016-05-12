#include "writer/verilog/state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"

static const char I[] = "        ";

namespace iroha {
namespace writer {
namespace verilog {

State::State(IState *i_state, Table *table)
  : i_state_(i_state), table_(table), transition_insn_(nullptr) {
  is_compound_cycle_ = DesignUtil::NumMultiCycleInsn(i_state) > 0;
  transition_insn_ = DesignUtil::FindTransitionInsn(i_state);
}

void State::Build() {
  ModuleTemplate *tmpl_ = table_->GetModuleTemplate();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  for (auto *insn : i_state_->insns_) {
    auto *res = insn->GetResource();
    unique_ptr<Resource> builder(Resource::Create(*res, *table_));
    if (builder.get() != nullptr) {
      builder->BuildInsn(insn, this);
    }
    if (!resource::IsSet(*res->GetClass())) {
      CopyResults(insn, true, ws);
    }
  }
}

void State::Write(ostream &os) {
  os << I << "`" << table_->StateName(i_state_->GetId()) << ": begin\n";
  os << StateBodySectionContents();
  for (auto *insn : i_state_->insns_) {
    auto *res = insn->GetResource();
    if (!resource::IsSet(*res->GetClass()) &&
	!resource::IsSiblingTask(*res->GetClass())) {
      CopyResults(insn, false, os);
    }
  }
  WriteTransition(os);
  os << I << "end\n";
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
    os << InsnWriter::RegisterName(*oreg);
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
      table_->IsTask()) {
    os << I << "  " << sv << " <= `"
       << table_->StateName(Task::kTaskEntryStateId)
       << ";\n";
    return;
  }
  if (transition_insn_ == nullptr ||
      transition_insn_->target_states_.size() == 0) {
    return;
  }
  if (transition_insn_->target_states_.size() == 1) {
    os << I << "  " << sv << " <= `"
       << table_->StateName(transition_insn_->target_states_[0]->GetId())
       << ";\n";
    return;
  }
  IRegister *cond = transition_insn_->inputs_[0];
  os << I << "  if (" << cond->GetName() << ") begin\n"
     << I << "    " << sv << " <= "
     << "`" << table_->StateName(transition_insn_->target_states_[1]->GetId())
     << ";\n"
     << I << "  end else begin\n"
     << I << "    " << sv << " <= "
     << "`" << table_->StateName(transition_insn_->target_states_[0]->GetId())
     << ";\n"
     << I << "  end\n";
}

void State::WriteTransition(ostream &os) {
  if (is_compound_cycle_) {
    os << I << "  if (";
    vector<string> sts;
    for (IInsn *insn : i_state_->insns_) {
      if (!DesignUtil::IsMultiCycleInsn(insn)) {
	continue;
      }
      IResource *res = insn->GetResource();
      string st = InsnWriter::MultiCycleStateName(*res);
      st = "(" + st + " == 3)";
      unique_ptr<Resource> builder(Resource::Create(*res, *table_));
      string ready = builder->ReadySignal(insn);
      if (!ready.empty()) {
	st = "(" + st + " || " + ready + ")";
      }
      sts.push_back(st);
    }
    os << Util::Join(sts, " && ");
    os << ") begin\n";
    for (IInsn *insn : i_state_->insns_) {
      if (DesignUtil::IsMultiCycleInsn(insn)) {
	string st = InsnWriter::MultiCycleStateName(*insn->GetResource());
	os << I << "    " << st << " <= " << "0;\n";
      }
    }
  }
  WriteTransitionBody(os);
  if (is_compound_cycle_) {
    os << I << "  end\n";
  }
}

void State::WriteTaskEntry(Table *tab, ostream &os) {
  os << I << "`" << tab->StateName(Task::kTaskEntryStateId) << ": begin\n";
  os << I << "  if (" << Task::TaskEnablePin(*tab->GetITable(),  nullptr) << ") begin\n"
     << I << "    " << tab->StateVariable() << " <= `"
     << tab->InitialStateName() << ";\n"
     << tab->TaskEntrySectionContents()
     << I << "  end\n"
     << I << "end\n";
}

ostream &State::StateBodySectionStream() const {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  return tmpl->GetStream(StateBodySectionName());
}

string State::StateBodySectionContents() const {
  ModuleTemplate *tmpl = table_->GetModuleTemplate();
  return tmpl->GetContents(StateBodySectionName());
}

string State::StateBodySectionName() const {
  return kStateBodySection +
    Util::Itoa(table_->GetITable()->GetId()) +
    ":" + Util::Itoa(i_state_->GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
