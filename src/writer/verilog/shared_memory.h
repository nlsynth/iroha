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

  static string GetName(const IResource &res);
  // Pin names (addr, rdata, wdata, wen) are used to connect
  // port 1 (of 0 and 1) or external sram.
  static string MemoryAddrPin(const IResource &res, int nth_port,
			      const IResource *accessor);
  static string MemoryRdataPin(const IResource &res, int nth_port);
  static string MemoryWdataPin(const IResource &res, int nth_port,
			       const IResource *accessor);
  static string MemoryWenPin(const IResource &res, int nth_port,
			     const IResource *accessor);
  static string MemoryWenReg(const IResource &res, int nth_port);
  // Buffer to capture the rdata by readers.
  // (the data may change if other readers start reading).
  static string MemoryRdataBuf(const IResource &res, const IResource *accessor);

private:
  void BuildMemoryResource();
  void BuildMemoryInstance();
  void BuildExternalMemoryConnection();
  void BuildAccessWireAll(vector<const IResource *> &acccessors);
  void BuildAck();

  static string MemoryPinPrefix(const IResource &res,
				const IResource *accessor);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_memory_h_
