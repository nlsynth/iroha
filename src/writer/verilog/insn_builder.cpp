#include "writer/verilog/insn_builder.h"

#include "writer/verilog/insn_writer.h"
#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace writer {
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

void InsnBuilder::Mapped() {
  IResource *res = insn_->GetResource();
  auto *params = res->GetParams();
  if (params->GetMappedName() == "mem") {
    string res_id = Util::Itoa(res->GetId());
    const string &opr = insn_->GetOperand();
    if (opr == "sram_read_data") {
      os_ << "  assign " << InsnWriter::InsnOutputWireName(*insn_, 0)
	  << " = sram_rdata_" << res_id << ";\n";
    }
  }
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
