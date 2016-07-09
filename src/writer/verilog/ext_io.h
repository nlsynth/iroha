// -*- C++ -*-
#ifndef _writer_verilog_ext_io_h_
#define _writer_verilog_ext_io_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ExtIO : public Resource {
public:
  ExtIO(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  void BuildExtInputInsn(IInsn *insn);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ext_io_h_
