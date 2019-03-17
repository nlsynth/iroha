// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_ext_writer_h_
#define _writer_verilog_shared_reg_ext_writer_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedRegExtWriter : public Resource {
public:
  SharedRegExtWriter(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_ext_writer_h_
