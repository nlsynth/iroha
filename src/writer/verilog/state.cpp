#include "writer/verilog/state.h"

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
}

void State::Build() {
  ModuleTemplate *tmpl_ = table_->GetModuleTemplate();
  ostream &ws = tmpl_->GetStream(kInsnWireValueSection);
  for (auto *insn : i_state_->insns_) {
    if (insn->GetResource()->GetClass()->GetName() == resource::kTransition) {
      transition_insn_ = insn;
    }
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
    }
    if (rc_name != resource::kSet) {
      CopyResults(insn, true, ws);
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

void State::WriteTransition(ostream &os) {
  if (transition_insn_ == nullptr ||
      transition_insn_->target_states_.size() == 0) {
    return;
  }
  const string &sv = table_->StateVariable();
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

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
