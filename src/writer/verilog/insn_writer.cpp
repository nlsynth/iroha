#include "writer/verilog/insn_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_params.h"

static const char I[] = "          ";

namespace iroha {
namespace verilog {

InsnWriter::InsnWriter(const IInsn *insn, ostream &os)
  : insn_(insn), os_(os) {
}

void InsnWriter::ExtInput() {
  auto *res = insn_->GetResource();
  auto *params = res->GetParams();
  string input_port;
  int width;
  params->GetExtInputPort(&input_port, &width);
  os_ << I << insn_->outputs_[0]->GetName() << " <= "
      << input_port << ";\n";
}

void InsnWriter::ExtOutput() {
  auto *res = insn_->GetResource();
  auto *params = res->GetParams();
  string output_port;
  int width;
  params->GetExtOutputPort(&output_port, &width);
  os_ << I << output_port << " <= "
      << insn_->inputs_[0]->GetName();
  os_ << ";\n";
}

void InsnWriter::Set() {
  os_ << I << insn_->outputs_[0]->GetName() << " <= "
      << RegisterName(*insn_->inputs_[0]) << ";\n";
}

void InsnWriter::BinOp() {
  os_ << I << insn_->outputs_[0]->GetName() << " <= "
      << ResourceName(*insn_->GetResource()) << "_d0;\n";
}

string InsnWriter::RegisterName(const IRegister &reg) {
  if (reg.IsConst()) {
    return Util::Itoa(reg.GetInitialValue().value_);
  } else {
    return reg.GetName();
  }
}

string InsnWriter::ResourceName(const IResource &res) {
  return res.GetClass()->GetName() + "_" + Util::Itoa(res.GetId());
}

}  // namespace verilog
}  // namespace iroha
