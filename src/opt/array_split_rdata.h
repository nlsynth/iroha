// -*- C++ -*-
#ifndef _opt_array_split_rdata_h_
#define _opt_array_split_rdata_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

class ArraySplitRData : public Phase {
public:
  virtual ~ArraySplitRData();

  static Phase *Create();
};

}  // namespace opt
}  // namespace iroha

#endif //  _opt_array_elimination_h_
