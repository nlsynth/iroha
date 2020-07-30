// -*- C++ -*-
#ifndef _writer_verilog_sram_if_h_
#define _writer_verilog_sram_if_h_
#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SramIf : public Resource {
public:
  SramIf(const IResource &res, const Table &table);
  virtual ~SramIf() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;
  virtual void CollectNames(Names *names) override;

private:
  void AddPorts(Module *mod);
  string AckReg();
  string NotifyReg();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_sram_if_h_
