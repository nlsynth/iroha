// -*- C++ -*-
#ifndef _writer_verilog_state_h_
#define _writer_verilog_state_h_

#include "iroha/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class Table;

class State {
public:
  State(IState *state, Table *table);

  void Build();
  void Write(ostream &os);
  const IState *GetIState() const;

private:
  void WriteInsn(const IInsn *insn, ostream &os);
  void WriteTransition(ostream &os);
  void CopyResults(const IInsn *insn, bool to_wire, ostream &os);

  IState *i_state_;
  Table *table_;
  IInsn *transition_insn_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_state_h_
