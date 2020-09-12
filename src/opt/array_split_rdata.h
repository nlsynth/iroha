// -*- C++ -*-
#ifndef _opt_array_split_rdata_h_
#define _opt_array_split_rdata_h_

#include "opt/pass.h"

namespace iroha {
namespace opt {

class ArraySplitRData : public Phase {
 public:
  virtual ~ArraySplitRData();

  static Phase *Create();

 private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace opt
}  // namespace iroha

#endif  //  _opt_array_elimination_h_
