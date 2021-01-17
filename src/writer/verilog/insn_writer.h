// -*- C++ -*-
#ifndef _writer_verilog_insn_writer_h_
#define _writer_verilog_insn_writer_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class InsnWriter {
 public:
  static string RegisterValue(const IRegister &reg, Names *names);
  static string RegisterValueWithConstWidth(const IRegister &reg, int width,
                                            Names *names);
  static string ConstValue(const IRegister &reg);
  static string CustomResourceName(const string &name, const IResource &res);
  static string ResourceName(const IResource &res);
  static string InsnOutputWireName(const IInsn &insn, int nth);
  static string InsnConstWireName(const IInsn &insn);
  // Up to each resource.
  static string InsnSpecificWireName(const IInsn &insn);
  static string MultiCycleStateName(const IResource &res);
  // Original width to Target width.
  static string AdjustWidth(const string &val, int ow, int tw);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_insn_writer_h_
