// -*- C++ -*-
#ifndef _writer_verilog_dataflow_state_h_
#define _writer_verilog_dataflow_state_h_

#include "writer/verilog/common.h"
#include "writer/verilog/state.h"

namespace iroha {
namespace writer {
namespace verilog {

class DataFlowState : public State {
public:
  DataFlowState(IState *state, Table *table);
  virtual ~DataFlowState();

  virtual void Write(ostream &os);
  static string StateVariable(const IState *st);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_dataflow_state_h_
