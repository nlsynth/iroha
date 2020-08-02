// -*- C++ -*-
#ifndef _writer_verilog_ticker_accessor_h_
#define _writer_verilog_ticker_accessor_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class TickerAccessor : public Resource {
public:
  TickerAccessor(const IResource &res, const Table &table);
  virtual ~TickerAccessor() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static bool UseDecrement(const IResource *accessor);

private:
  void CollectDecrementCallers(map<IState *, IInsn *> *callers);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ticker_accessor_h_
