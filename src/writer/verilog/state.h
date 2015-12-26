// -*- C++ -*-
#ifndef _writer_verilog_state_h_
#define _writer_verilog_state_h_

#include "iroha/common.h"

namespace iroha {
namespace verilog {

class Table;

class State {
public:
  State(IState *state, Table *table);

  void Build();
  void Write(ostream &os);

private:
  void WriteInsn(const IInsn *insn, ostream &os);
  void WriteTransition(ostream &os);

  IState *i_state_;
  Table *table_;
  IInsn *transition_insn_;
};

}  // namespace verilog
}  // namespace iroha

#endif  // _writer_verilog_state_h_
