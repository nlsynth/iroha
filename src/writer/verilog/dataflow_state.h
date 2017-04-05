// -*- C++ -*-
#ifndef _writer_verilog_dataflow_state_h_
#define _writer_verilog_dataflow_state_h_

#include "writer/verilog/common.h"
#include "writer/verilog/state.h"

namespace iroha {
namespace writer {
namespace verilog {

struct DataFlowStateTransition {
  DataFlowState *to;
  IState *to_raw;
  DataFlowState *from;
  // empty for unconditional.
  string cond;
};

class DataFlowState : public State {
public:
  DataFlowState(IState *state, Table *table, Names *names);
  virtual ~DataFlowState();

  virtual void Write(ostream &os);
  static string StateVariable(const IState *st);
  void BuildIncomingTransitions(const vector<DataFlowStateTransition> &trs);

  vector<DataFlowStateTransition> GetTransitions();

private:
  string StartCondition(IInsn *insn);

  string incoming_transitions_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_dataflow_state_h_
