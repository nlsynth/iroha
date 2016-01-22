// -*- C++ -*-
#ifndef _writer_verilog_insn_builder_h_
#define _writer_verilog_insn_builder_h_

#include "iroha/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class State;

class InsnBuilder {
public:
  InsnBuilder(const IInsn *insn, ostream &os);

  void ExtInput();
  void ExclusiveBinOp();
  void LightBinOp();
  void BitArrangeOp();

private:
  const IInsn *insn_;
  ostream &os_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_insn_builder_h_
