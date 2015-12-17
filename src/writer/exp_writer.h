// -*- C++ -*-
#ifndef _writer_exp_writer_h_
#define _writer_exp_writer_h_

#include "iroha/common.h"

namespace iroha {

class ExpWriter {
public:
  ExpWriter(const IDesign *design, ostream &os);

  void Write();

private:
  void WriteModule(const IModule *mod);
  void WriteTable(const ITable *tab);
  void WriteState(const IState *st);
  void WriteInsn(const IInsn *insn);
  void WriteRegisters(const ITable *tab);
  void WriteInsnParams(const vector<IRegister *> &regs);

  const IDesign *design_;
  ostream &os_;
};

}  // namespace iroha

#endif  // _writer_exp_writer_h_
