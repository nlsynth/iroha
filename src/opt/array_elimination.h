// -*- C++ -*-
#ifndef _opt_array_elimination_h_
#define _opt_array_elimination_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {

class ArrayElimination : public Pass {
 public:
  virtual ~ArrayElimination();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);

  IRegister *GetRegister(IResource *array_res, IRegister *index_reg);
  map<tuple<IResource *, int>, IRegister *> fixed_regs_;
};

}  // namespace opt
}  // namespace iroha

#endif  //  _opt_array_elimination_h_
