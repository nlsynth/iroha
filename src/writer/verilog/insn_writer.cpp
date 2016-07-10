#include "writer/verilog/insn_writer.h"

#include "iroha/logging.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"
#include "writer/verilog/task.h"

static const char I[] = "          ";

namespace iroha {
namespace writer {
namespace verilog {

InsnWriter::InsnWriter(const IInsn *insn, const State *st,
		       ostream &os)
  : insn_(insn), st_(st), os_(os) {
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

void InsnWriter::Mapped() {
}

string InsnWriter::InsnOutputWireName(const IInsn &insn, int nth) {
  return "insn_o_" + Util::Itoa(insn.GetResource()->GetTable()->GetId()) + "_"
    + Util::Itoa(insn.GetId()) + "_" + Util::Itoa(nth);
}

string InsnWriter::MultiCycleStateName(const IResource &res) {
  return "st_res_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
