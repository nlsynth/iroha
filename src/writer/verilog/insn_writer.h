// -*- C++ -*-
#ifndef _writer_verilog_insn_writer_h_
#define _writer_verilog_insn_writer_h_

#include "iroha/common.h"

namespace iroha {
namespace verilog {


class InsnWriter {
public:
  InsnWriter(const IInsn *insn, ostream &os);

  void ExtInput();
  void ExtOutput();
  void Set();
  void BinOp();

  static string RegisterName(const IRegister &reg);
  static string ResourceName(const IResource &res);

private:
  const IInsn *insn_;
  ostream &os_;
};

}  // namespace verilog
}  // namespace iroha

#endif  // _writer_verilog_insn_writer_h_
