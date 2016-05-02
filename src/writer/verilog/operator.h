// -*- C++ -*-
#ifndef _writer_verilog_operator_h_
#define _writer_verilog_operator_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class Operator : public Resource {
public:
  Operator(const IResource &res, const Table &table);

  virtual void BuildResource();
  virtual void BuildInsn(IInsn *insn, State *st);

private:
  void BuildExclusiveBinOp();
  void BuildLightBinOpInsn(IInsn *insn);
  void BuildBitArrangeOpInsn(IInsn *insn);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_operator_h_
