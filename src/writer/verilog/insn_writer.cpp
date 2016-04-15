#include "writer/verilog/insn_writer.h"

#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

static const char I[] = "          ";

namespace iroha {
namespace writer {
namespace verilog {

InsnWriter::InsnWriter(const IInsn *insn, const State *st,
		       ostream &os)
  : insn_(insn), st_(st), os_(os) {
}

void InsnWriter::ExtOutput() {
  auto *res = insn_->GetResource();
  auto *params = res->GetParams();
  string output_port;
  int width;
  params->GetExtOutputPort(&output_port, &width);
  os_ << I << output_port << " <= "
      << RegisterName(*insn_->inputs_[0]);
  os_ << ";\n";
}

void InsnWriter::Set() {
  os_ << I << insn_->outputs_[0]->GetName() << " <= "
      << RegisterName(*insn_->inputs_[0]) << ";\n";
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

void InsnWriter::SubModuleCall() {
  string st = MultiCycleStateName(*insn_);
  IResource *res = insn_->GetResource();
  string pin = Table::TaskControlPinPrefix(*res);
  os_ << I << "if (" << st << " == 0) begin\n"
      << I << "  if (" << pin << "_ack) begin\n"
      << I << "    " << pin << "_en <= 0;\n"
      << I << "    " << st << " <= 3;\n"
      << I << "  end else begin\n"
      << I << "  " << pin << "_en <= 1;\n"
      << I << "  end\n"
      << I << "end\n";
}

void InsnWriter::Mapped() {
  IResource *res = insn_->GetResource();
  auto *params = res->GetParams();
  if (params->GetMappedName() == "mem") {
    string res_id = Util::Itoa(res->GetId());
    const string &opr = insn_->GetOperand();
    if (opr == "sram_read_address" ||
	opr == "sram_write") {
      os_ << I << "sram_addr_" + res_id << " <= "
	  << RegisterName(*(insn_->inputs_[0]))
	  << ";\n";
    }
    if (opr == "sram_write") {
      os_ << I << "sram_wdata_" + res_id << " <= "
	  << RegisterName(*(insn_->inputs_[1]))
	  << ";\n";
    }
  }
}

string InsnWriter::InsnOutputWireName(const IInsn &insn, int nth) {
  return "insn_o_" + Util::Itoa(insn.GetResource()->GetTable()->GetId()) + "_"
    + Util::Itoa(insn.GetId()) + "_" + Util::Itoa(nth);
}

string InsnWriter::MultiCycleStateName(const IInsn &insn) {
  return "st_insn_" + Util::Itoa(insn.GetResource()->GetTable()->GetId())
    + "_" + Util::Itoa(insn.GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
