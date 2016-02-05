#include "writer/verilog/state.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "writer/module_template.h"
#include "writer/verilog/insn_builder.h"
#include "writer/verilog/insn_writer.h"
#include "writer/verilog/module.h"
#include "writer/verilog/table.h"

static const char I[] = "        ";

namespace iroha {
namespace writer {
namespace verilog {

State::State(IState *i_state, Table *table)
  : i_state_(i_state), table_(table), transition_insn_(nullptr) {
  is_multi_cycle_ = DesignUtil::IsMultiCycleState(i_state);
  transition_insn_ = DesignUtil::FindTransitionInsn(i_state);
}

void State::Build() {
  ModuleTemplate *tmpl_ = table_->GetModuleTemplate();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  for (auto *insn : i_state_->insns_) {
    auto *res = insn->GetResource();
    auto *rc = res->GetClass();
    const string &rc_name = rc->GetName();
    InsnBuilder builder(insn, ws);
    if (rc_name == resource::kExtInput) {
      builder.ExtInput();
    } else if (resource::IsLightBinOp(*rc)) {
      builder.LightBinOp();
    } else if (resource::IsBitArrangeOp(*rc)) {
      builder.BitArrangeOp();
    } else if (resource::IsMapped(*rc)) {
      builder.Mapped();
    }
    if (rc_name != resource::kSet) {
      CopyResults(insn, true, ws);
    }
    if (DesignUtil::IsMultiCycleInsn(insn)) {
      BuildMultiCycle(insn);
    }
  }
}

void State::Write(ostream &os) {
  os << I << "`" << table_->StateName(i_state_->GetId()) << ": begin\n";
  for (auto *insn : i_state_->insns_) {
    WriteInsn(insn, os);
  }
  WriteTransition(os);
  os << I << "end\n";
}

void State::BuildMultiCycle(const IInsn *insn) {
  ModuleTemplate *tmpl_ = table_->GetModuleTemplate();
  ostream &ws = tmpl_->GetStream(kInsnWireDeclSection);
  string w = InsnWriter::MultiCycleStateName(*insn);
  ws << "  reg [1:0] " << w << ";\n";
  ostream &is = tmpl_->GetStream(kInitialValueSection +
				 Util::Itoa(table_->GetITable()->GetId()));
  is << "      " << w << " <= 0;\n";
}

const IState *State::GetIState() const {
  return i_state_;
}

void State::WriteInsn(const IInsn *insn, ostream &os) {
  InsnWriter writer(insn, this, os);
  auto *res = insn->GetResource();
  auto *rc = res->GetClass();
  const string &rc_name = rc->GetName();
  if (rc_name == resource::kSet) {
    writer.Set();
  } else if (rc_name == resource::kExtOutput) {
    writer.ExtOutput();
  } else if (resource::IsExclusiveBinOp(*rc)) {
  } else if (resource::IsLightBinOp(*rc)) {
  } else if (resource::IsBitArrangeOp(*rc)) {
  } else if (rc_name == resource::kTransition) {
    // do nothing.
  } else if (rc_name == resource::kPrint) {
    writer.Print();
  } else if (rc_name == resource::kAssert) {
    writer.Assert();
  } else if (rc_name == resource::kSubModuleTaskCall) {
    writer.SubModuleCall();
  } else if (resource::IsMapped(*rc)) {
    writer.Mapped();
  } else {
    // LOG(FATAL) << "Unsupported resource class:" << rc_name;
  }
  if (rc_name != resource::kSet) {
    CopyResults(insn, false, os);
  }
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
       << table_->InitialStateName()
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
  bool is_mc = DesignUtil::IsMultiCycleState(i_state_);
  if (is_mc) {
    os << I << "  if (";
    bool is_first = true;
    for (IInsn *insn : i_state_->insns_) {
      if (!DesignUtil::IsMultiCycleInsn(insn)) {
	continue;
      }
      if (!is_first) {
	os << " && ";
      }
      string st = InsnWriter::MultiCycleStateName(*insn);
      os << "(" << st << " == 3)";
      is_first = false;
    }
    os << ") begin\n";
  }
  for (IInsn *insn : i_state_->insns_) {
    if (DesignUtil::IsMultiCycleInsn(insn)) {
      string st = InsnWriter::MultiCycleStateName(*insn);
      os << I << "    " << st << " <= " << "0;\n";
    }
  }
  WriteTransitionBody(os);
  if (is_mc) {
    os << I << "  end\n";
  }
}

void State::WriteTaskEntry(Table *tab, ostream &os) {
  os << I << "`" << tab->StateName(Table::kTaskEntryStateId) << ": begin\n";
  os << I << "  if (" << tab->TaskEnablePin() << ") begin\n"
     << I << "    " << tab->StateVariable() << " <= `"
     << tab->InitialStateName() << ";\n";
  os << I << "  end\n"
     << I << "end\n";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
