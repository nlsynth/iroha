// -*- C++ -*-
#ifndef _writer_verilog_ext_io_accessor_h_
#define _writer_verilog_ext_io_accessor_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ExtIOAccessor : public Resource {
public:
  ExtIOAccessor(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ext_io_accessor_h_
