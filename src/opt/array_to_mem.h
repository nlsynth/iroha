// -*- C++ -*-
#ifndef _opt_array_to_mem_h_
#define _opt_array_to_mem_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class ArrayToMem : public Phase {
public:
  virtual ~ArrayToMem();

  static Phase *Create();

private:
  virtual bool ApplyForTable(const string &key, ITable *table);

  IResource *GetResource(IResource *array);
  void AddMemInsn(IResource *mem, IState *st, IInsn *array_insn);
  void DeleteArrayInsn(IState *st, IInsn *array_insn);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_array_to_mem_h_
