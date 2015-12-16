// -*- C++ -*-
#ifndef _writer_exp_writer_h_
#define _writer_exp_writer_h_

#include "iroha/common.h"

namespace iroha {

class IDesign;
class IInsn;
class IModule;
class IState;
class ITable;

class ExpWriter {
public:
  ExpWriter(const IDesign *design, ostream &os);

  void Write();

private:
  void WriteModule(const IModule *mod);
  void WriteTable(const ITable *tab);
  void WriteState(const IState *st);
  void WriteInsn(const IInsn *insn);

  const IDesign *design_;
  ostream &os_;
};

}  // namespace iroha

#endif  // _writer_exp_writer_h_
