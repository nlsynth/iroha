// -*- C++ -*-
#ifndef _writer_verilog_insn_writer_h_
#define _writer_verilog_insn_writer_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class InsnWriter {
public:
  InsnWriter(const IInsn *insn, const State *st, ostream &os);

  void ExtOutput();
  void Set();
  void Print();
  void Assert();
  void Mapped();

  static string RegisterName(const IRegister &reg);
  static string ResourceName(const IResource &res);
  static string ChannelDataPort(const IChannel &ic);
  static string InsnOutputWireName(const IInsn &insn, int nth);
  static string MultiCycleStateName(const IResource &res);

private:
  const IInsn *insn_;
  const State *st_;
  ostream &os_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_insn_writer_h_
