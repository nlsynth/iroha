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

private:
  string TickerName();
  string BuildDecrement();
  bool HasDecrement();
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ticker_h_
