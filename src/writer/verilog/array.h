// -*- C++ -*-
#ifndef _writer_verilog_array_h_
#define _writer_verilog_array_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

// SharedMemoryReplica inherits this.
class ArrayResource : public Resource {
 public:
  ArrayResource(const IResource &res, const Table &table);
  virtual ~ArrayResource(){};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string SigName(const IResource &res, const string &sig);

 private:
  void BuildExternalSRAM();
  void BuildInternalSRAM();
  void BuildSRAMWrite();

 protected:
  string SigName(const string &sig);
  virtual IArray *GetArray();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_array_h_
