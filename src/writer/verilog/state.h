// -*- C++ -*-
#ifndef _writer_verilog_state_h_
#define _writer_verilog_state_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class State {
public:
  State(IState *state, Table *table);

  void Build();
  void Write(ostream &os);
  const IState *GetIState() const;
  static void WriteTaskEntry(Table *tab, ostream &os);

  ostream &StateBodySectionStream() const;
  string StateBodySectionContents() const;

private:
  void WriteInsn(const IInsn *insn, ostream &os);
  void WriteTransition(ostream &os);
  void WriteTransitionBody(ostream &os);
  void CopyResults(const IInsn *insn, bool to_wire, ostream &os);
  void BuildMultiCycle(const IInsn *insn);
  string StateBodySectionName() const;

  IState *i_state_;
  Table *table_;
  IInsn *transition_insn_;
  bool is_multi_cycle_;
  bool is_compound_cycle_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_state_h_
