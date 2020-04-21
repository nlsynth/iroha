// -*- C++ -*-
#ifndef _writer_verilog_array_rdata_h_
#define _writer_verilog_array_rdata_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

// 2nd clock of SRAM read phase.
// This can be used instead of ArrayResource to describe pipeline.
class ArrayRDataResource : public Resource {
public:
  ArrayRDataResource(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_array_rdata_h_
