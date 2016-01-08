#include "writer/verilog/insn_writer.h"

#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/state.h"

static const char I[] = "          ";

namespace iroha {
namespace verilog {

InsnWriter::InsnWriter(const IInsn *insn, const State *st, ostream &os)
  : insn_(insn), st_(st), os_(os) {
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

void InsnWriter::ExclusiveBinOp() {
  os_ << I << insn_->outputs_[0]->GetName() << " <= "
      << ResourceName(*insn_->GetResource()) << "_d0;\n";
}

void InsnWriter::LightBinOp() {
  os_ << I << insn_->outputs_[0]->GetName() << " <= "
      << RegisterName(*insn_->inputs_[0]) << " ";
  const string &rc = insn_->GetResource()->GetClass()->GetName();
  if (rc == resource::kXor) {
    os_ << "^";
  } else {
  }
  os_ << " " << RegisterName(*insn_->inputs_[1]) << ";\n";
}

void InsnWriter::BitArrangeOp() {
  const string &rc = insn_->GetResource()->GetClass()->GetName();
  if (rc == resource::kShift) {
    bool is_left = (insn_->GetOperand() == "left");
    const IValue &value = insn_->inputs_[1]->GetInitialValue();
    int amount = value.value_;
    os_ << I << insn_->outputs_[0]->GetName() << " <= "
	<< RegisterName(*insn_->inputs_[0]);
    if (is_left) {
      os_ << " << ";
    } else {
      os_ << " >> ";
    }
    os_ << amount << ";\n";
  }
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

string InsnWriter::ChannelDataPort(const IChannel &ic) {
  if (ic.GetReader() != nullptr) {
      if (ic.GetWriter() != nullptr) {
	return "channel_data_" + Util::Itoa(ic.GetId());
      } else {
	return "ext_rdata_" + Util::Itoa(ic.GetId());
      }
  } else {
    return "ext_wdata_" + Util::Itoa(ic.GetId());
  }
}

void InsnWriter::Print() {
  for (int i = 0; i < insn_->inputs_.size(); ++i) {
    IRegister *reg = insn_->inputs_[i];
    os_ << I << "$display(\"%d\", " << RegisterName(*reg) << ");\n";
  }
}

void InsnWriter::Assert() {
  os_ << I << "if (!(";
  for (int i = 0; i < insn_->inputs_.size(); ++i) {
    if (i > 0) {
      os_ << " && ";
    }
    IRegister *reg = insn_->inputs_[i];
    os_ << RegisterName(*reg);
  }
  os_ << ")) begin\n";
  os_ << I << "  $display(\"ASSERTION FAILURE: "
      << st_->GetIState()->GetId() <<"\");\n";
  os_ << I << "end\n";
}

}  // namespace verilog
}  // namespace iroha
