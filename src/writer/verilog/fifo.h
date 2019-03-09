// -*- C++ -*-
#ifndef _writer_verilog_fifo_h_
#define _writer_verilog_fifo_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class Fifo : public Resource {
public:
  Fifo(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string RReq(const IResource &res, const IResource *accessor);
  static string RAck(const IResource &res, const IResource *accessor);
  static string RData(const IResource &res);
  static string RDataBuf(const IResource &reader);
  static string WReq(const IResource &res, const IResource *accessor);
  static string WAck(const IResource &res, const IResource *accessor);
  static string WData(const IResource &res, const IResource *accessor);
  static string WNoWait(const IResource &res, const IResource *accessor);
  static string GetName(const IResource &res);
  static string GetNameRW(const IResource &res, bool is_write);

private:
  string WritePtr();
  string ReadPtr();
  string ReadPtrBuf();
  string Full();
  string Empty();
  string WEn();
  string WReqWire();
  string WAckWire();
  string WAckReg();
  string WDataWire();
  string WNoWaitWire();
  string RReqWire();
  string RAckWire();
  string RAckReg();
  string RDataWire();

  static string PinPrefix(const IResource &res, const IResource *accessor);

  void BuildMemoryInstance();
  void BuildWires();
  void BuildWriterConnections();
  void BuildReaderConnections();
  void BuildController();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_fifo_h_
