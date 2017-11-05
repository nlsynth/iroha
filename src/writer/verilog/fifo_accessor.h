// -*- C++ -*-
#ifndef _writer_verilog_fifo_accessor_h_
#define _writer_verilog_fifo_accessor_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class FifoAccessor : public Resource {
public:
  FifoAccessor(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  void BuildReader();
  void BuildWriter();
  void BuildReadInsn(IInsn *insn, State *st);
  void BuildWriteInsn(IInsn *insn, State *st);
  void BuildReq(bool is_writer);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_fifo_accessor_h_
