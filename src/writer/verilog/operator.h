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
  virtual ~Operator() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  void BuildExclusiveBinOp();
  void BuildExclusiveBinOpInsn(IInsn *insn);
  void BuildLightBinOpInsn(IInsn *insn);
  void BuildLightUniOpInsn(IInsn *insn);
  void BuildBitShiftOpInsn(IInsn *insn);
  void BuildBitSelInsn(IInsn *insn);
  void BuildBitConcatInsn(IInsn *insn);
  void BuildSelectInsn(IInsn *insn);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_operator_h_
