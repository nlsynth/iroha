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

  static string MemoryAddrPin(const IResource &res, int nth_port,
			      const IResource *accessor);
  static string MemoryRdataPin(const IResource &res, int nth_port);
  static string MemoryWdataPin(const IResource &res, int nth_port,
			       const IResource *accessor);
  static string MemoryWenPin(const IResource &res, int nth_port,
			     const IResource *accessor);
  static string MemoryReqPin(const IResource &res, const IResource *accessor);
  static string MemoryAckPin(const IResource &res, const IResource *accessor);
  // Buffer to capture the rdata by readers.
  // (the data may change if other readers start reading).
  static string MemoryRdataBuf(const IResource &res, const IResource *accessor);
  // Wire just to connect to the SRAM instance.
  // (this simplifies the code if there's no readers).
  static string MemoryRdataRawPin(const IResource &res);

private:
  void BuildMemoryResource();
  void BuildMemoryInstance();
  void BuildExternalMemoryConnection();
  void BuildAccessWireAll(vector<const IResource *> &acccessors);

  static string MemoryPinPrefix(const IResource &res,
				const IResource *accessor);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_memory_h_
