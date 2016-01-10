// -*- C++ -*-
#ifndef _opt_bb_collector_h_
#define _opt_bb_collector_h_

#include "iroha/common.h"

namespace iroha {
namespace opt {

class BB;
class BBSet;

class BBCollector {
public:
  BBCollector(ITable *table);
  BBSet *Create();

private:
  ITable *table_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_bb_collector_h_
