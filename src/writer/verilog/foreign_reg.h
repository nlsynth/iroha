// -*- C++ -*-
#ifndef _writer_verilog_foreign_reg_h_
#define _writer_verilog_foreign_reg_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class ForeignReg : public Resource {
public:
  ForeignReg(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static void BuildPorts(const RegConnectionInfo &ri, Ports *ports, Names *names);
  static void BuildChildWire(const RegConnectionInfo &ri, Names *names, ostream &os);
  static void BuildRegWire(const RegConnectionInfo &ri, Module *module);

private:
  static void AddChildWire(IRegister *reg, Names *names, ostream &os);
  static string ForeignRegName(const IRegister *reg, Names *names);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_foreign_reg_h_
