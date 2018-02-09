// -*- C++ -*-
#ifndef _writer_verilog_state_h_
#define _writer_verilog_state_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class State {
public:
  State(IState *state, Table *table, Names *names);
  virtual ~State();

  void Build();
  virtual void Write(ostream &os);
  const IState *GetIState() const;
  static void WriteTaskEntry(Table *tab, ostream &os);
  bool IsCompoundCycle() const;
  Names *GetNames() const;

  ostream &StateBodySectionStream() const;
  string StateBodySectionContents() const;
  ostream &StateTransitionSectionStream() const;
  string StateTransitionSectionContents() const;

private:
  void WriteTransition(ostream &os);
  void WriteTransitionBody(ostream &os);
  void CopyResults(const IInsn *insn, bool to_wire, ostream &os);
  string StateBodySectionName() const;
  string StateTransitionSectionName() const;
  string MultiCycleTransitionCond();

protected:
  void WriteStateBody(ostream &os);
  void ClearMultiCycleState(ostream &os);

  IState *i_state_;
  Table *table_;
  Names *names_;
  IInsn *transition_insn_;
  bool is_compound_cycle_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_state_h_
