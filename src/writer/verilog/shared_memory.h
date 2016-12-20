// -*- C++ -*-
#ifndef _writer_verilog_shared_memory_h_
#define _writer_verilog_shared_memory_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedMemory : public Resource {
public:
  SharedMemory(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  void BuildMemoryReaderResource();
  void BuildMemoryWriterResource();
  void BuildMemoryResource();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_memory_h_
