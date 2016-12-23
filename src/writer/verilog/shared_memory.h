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
  void BuildMemoryInstance();

  static string MemoryPinPrefix(const IResource &res,
				const IResource *accessor);
  static string MemoryAddrPin(const IResource &res, const IResource *accessor);
  static string MemoryReqPin(const IResource &res, const IResource *accessor);
  static string MemoryAckPin(const IResource &res, const IResource *accessor);
  static string MemoryRdataPin(const IResource &res);
  static string MemoryWdataPin(const IResource &res, const IResource *accessor);
  static string MemoryWenPin(const IResource &res, const IResource *accessor);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_memory_h_
