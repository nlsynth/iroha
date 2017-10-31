// -*- C++ -*-
#ifndef _writer_verilog_fifo_h_
#define _writer_verilog_fifo_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

// Fifo can have depth and will supersede Channel (WIP).
class Fifo : public Resource {
public:
  Fifo(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  string WritePtr();
  string ReadPtr();
  string PinPrefix();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_fifo_h_
