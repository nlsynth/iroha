// -*- C++ -*-
#ifndef _writer_verilog_fifo_h_
#define _writer_verilog_fifo_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

// Fifo can have depth and will supersede Channel (WIP).
class Fifo : public Resource {
public:
  Fifo(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static void AddAccessorSignals(const IModule *imod, const Table *tab,
				 const IResource *accessor, bool wire_only);

  static string RReq(const IResource &res, const IResource *accessor);
  static string RAck(const IResource &res, const IResource *accessor);
  static string RData(const IResource &res);
  static string RDataBuf(const IResource &reader);
  static string WReq(const IResource &res, const IResource *accessor);
  static string WAck(const IResource &res, const IResource *accessor);
  static string WData(const IResource &res, const IResource *accessor);

private:
  string WritePtr();
  string ReadPtr();
  string Full();
  string Empty();
  static string PinPrefix(const IResource &res, const IResource *accessor);
  void BuildMemoryInstance();
  void BuildWires();
  void BuildAccessConnectionsAll();
  void BuildAccessConnection(IResource *accessor);
  void AddWire(const IModule *imod, IResource *caller);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_fifo_h_
