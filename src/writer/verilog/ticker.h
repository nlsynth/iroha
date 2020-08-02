// -*- C++ -*-
#ifndef _writer_verilog_ticker_h_
#define _writer_verilog_ticker_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class Ticker : public Resource {
public:
  Ticker(const IResource &res, const Table &table);
  virtual ~Ticker() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string TickerName(const IResource &res);

private:
  string TickerName();
  string BuildSelfDecrement();
  string BuildAccessorDecrement();
  bool HasSelfDecrement();
  bool HasAccessorDecrement();
  void BuildAccessorWire();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ticker_h_
