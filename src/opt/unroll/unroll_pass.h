// -*- C++ -*-
#ifndef _opt_unroll_unroll_pass_h_
#define _opt_unroll_unroll_pass_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {
namespace unroll {

class UnrollPass : public Pass {
 public:
  virtual ~UnrollPass();

  static Pass *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace unroll
}  // namespace opt
}  // namespace iroha

#endif  // _opt_unroll_unroll_pass_h_
