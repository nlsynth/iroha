// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_h_
#define _writer_verilog_shared_reg_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedReg : public Resource {
public:
  SharedReg(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string PortName(const IResource &res);

  static void BuildPorts(const PortConnectionInfo &pi, Ports *ports);
  static void BuildChildWire(const PortConnectionInfo &pi, ostream &os);
  static void BuildRootWire(const PortConnectionInfo &pi, Module *module);

private:
  static void AddChildWire(IResource *res, ostream &os);

  int width_;
  string output_port_;
  bool has_default_output_value_;
  int default_output_value_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_h_
