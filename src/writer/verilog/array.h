// -*- C++ -*-
#ifndef _writer_verilog_mapped_h_
#define _writer_verilog_mapped_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ArrayResource : public Resource {
public:
  ArrayResource(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

private:
  void BuildMemInsn(IInsn *insn, State *st);
  void BuildExternalSRAM();
  void BuildInternalSRAM();
  void BuildSRAMWrite();

  string SigName(const string &sig);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_mapped_h_
