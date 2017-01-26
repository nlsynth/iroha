// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_accessor_h_
#define _writer_verilog_shared_reg_accessor_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedRegAccessor : public Resource {
public:
  SharedRegAccessor(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  void BuildSharedRegWriterResource();
  void BuildWriteWire(const IResource *writer);
  void AddWritePort(const IModule *imod, const IResource *writer,
		    bool upward);

  int width_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_accessor_h_
