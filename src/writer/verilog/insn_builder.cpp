#include "writer/verilog/insn_builder.h"

#include "writer/verilog/insn_writer.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace verilog {

InsnBuilder::InsnBuilder(const IInsn *insn, ostream &os)
  : insn_(insn), os_(os) {
}

void InsnBuilder::ExtInput() {
  auto *res = insn_->GetResource();
  auto *params = res->GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  os_ << "  assign " << InsnWriter::InsnOutputWireName(*insn_, 0)
      << " = "
      << input_port << ";\n";
}

void InsnBuilder::ExclusiveBinOp() {
  os_ << "  assign " << InsnWriter::InsnOutputWireName(*insn_, 0)
      << " = "
      << InsnWriter::ResourceName(*insn_->GetResource()) << "_d0;\n";
}

void InsnBuilder::LightBinOp() {
  os_ << "  assign " << InsnWriter::InsnOutputWireName(*insn_, 0)
      << " = " << InsnWriter::RegisterName(*insn_->inputs_[0]) << " ";
  const string &rc = insn_->GetResource()->GetClass()->GetName();
  if (rc == resource::kBitAnd) {
    os_ << "&";
  } else if (rc == resource::kBitOr) {
    os_ << "|";
  } else if (rc == resource::kBitXor) {
    os_ << "^";
  } else {
    LOG(FATAL) << "Unknown LightBinOp: " << rc;
  }
  os_ << " " << InsnWriter::RegisterName(*insn_->inputs_[1]) << ";\n";
}

void InsnBuilder::BitArrangeOp() {
  const string &rc = insn_->GetResource()->GetClass()->GetName();
  if (rc == resource::kShift) {
    bool is_left = (insn_->GetOperand() == "left");
    const IValue &value = insn_->inputs_[1]->GetInitialValue();
    int amount = value.value_;
    os_ << "  assign " << InsnWriter::InsnOutputWireName(*insn_, 0)
	<< " = "
	<< InsnWriter::RegisterName(*insn_->inputs_[0]);
    if (is_left) {
      os_ << " << ";
    } else {
      os_ << " >> ";
    }
    os_ << amount << ";\n";
  }
}

}  // namespace verilog
}  // namespace iroha
