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

  static string RReq(const IResource &res, const IResource *accessor);
  static string RAck(const IResource &res, const IResource *accessor);
  static string RData(const IResource &res);
  static string WReq(const IResource &res, const IResource *accessor);
  static string WAck(const IResource &res, const IResource *accessor);
  static string WData(const IResource &res, const IResource *accessor);

private:
  string WritePtr();
  string ReadPtr();
  static string PinPrefix(const IResource &res, const IResource *accessor);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_fifo_h_
