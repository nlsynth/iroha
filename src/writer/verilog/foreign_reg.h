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

  static string RegPrefix(const ITable &writer, const IRegister &reg);
  static void BuildForeignRegisters(const Table &tab);
  static void BuildPorts(const RegConnectionInfo &ri, Ports *ports);
  static void BuildChildWire(const RegConnectionInfo &ri, ostream &os);
  static void AddChildWire(IRegister *reg, ostream &os);
  static void BuildRegWire(const RegConnectionInfo &ri, Module *module);
  static string ForeignRegName(const IRegister *reg);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_foreign_reg_h_
