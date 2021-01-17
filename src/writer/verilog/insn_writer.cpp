#include "writer/verilog/insn_writer.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "writer/names.h"
#include "writer/verilog/state.h"
#include "writer/verilog/table.h"

static const char I[] = "          ";

namespace iroha {
namespace writer {
namespace verilog {

string InsnWriter::RegisterValue(const IRegister &reg, Names *names) {
  if (reg.IsConst()) {
    int w = reg.value_type_.GetWidth();
    if (w == 0) {
      w = 1;
    }
    return Util::Itoa(w) + "'d" +
           Util::ULLtoA(reg.GetInitialValue().GetValue0());
  } else {
    return names->GetRegName(reg);
  }
}

string InsnWriter::RegisterValueWithConstWidth(const IRegister &reg, int width,
                                               Names *names) {
  if (reg.IsConst()) {
    if (width == 0) {
      width = 1;
    }
    return Util::Itoa(width) + "'d" +
           Util::ULLtoA(reg.GetInitialValue().GetValue0());
  }
  return RegisterValue(reg, names);
}

string InsnWriter::ConstValue(const IRegister &reg) {
  CHECK(reg.IsConst());
  return Util::Itoa(reg.GetInitialValue().GetValue0());
}

string InsnWriter::CustomResourceName(const string &name,
                                      const IResource &res) {
  return name + "_" + Util::Itoa(res.GetTable()->GetId()) + "_" +
         Util::Itoa(res.GetId());
}

string InsnWriter::ResourceName(const IResource &res) {
  return CustomResourceName(res.GetClass()->GetName(), res);
}

string InsnWriter::InsnOutputWireName(const IInsn &insn, int nth) {
  return "insn_o_" + Util::Itoa(insn.GetResource()->GetTable()->GetId()) + "_" +
         Util::Itoa(insn.GetId()) + "_" + Util::Itoa(nth);
}

string InsnWriter::InsnConstWireName(const IInsn &insn) {
  return "insn_c_" + Util::Itoa(insn.GetResource()->GetTable()->GetId()) + "_" +
         Util::Itoa(insn.GetId());
}

string InsnWriter::InsnSpecificWireName(const IInsn &insn) {
  return "insn_" + Util::Itoa(insn.GetResource()->GetTable()->GetId()) + "_" +
         Util::Itoa(insn.GetId());
}

string InsnWriter::MultiCycleStateName(const IResource &res) {
  return "st_res_" + Util::Itoa(res.GetTable()->GetId()) + "_" +
         Util::Itoa(res.GetId());
}

string InsnWriter::AdjustWidth(const string &val, int ow, int tw) {
  if (ow == tw) {
    return val;
  }
  if (ow > tw) {
    return val + "[" + Util::Itoa(tw - 1) + ":0]";
  }
  int d = tw - ow;
  return "{" + Util::Itoa(d) + "'b0, " + val + "}";
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
