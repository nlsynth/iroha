// -*- C++ -*-
#ifndef _opt_simple_bb_merge_h_
#define _opt_simple_bb_merge_h_

#include "opt/phase.h"

namespace iroha {
namespace opt {

// WIP. Merges empty BB just to jump the previous BB.
class SimpleBBMerge : public Phase {
public:
  virtual ~SimpleBBMerge();

  static Phase *Create();

private:
  virtual bool ApplyForTable(const string &key, ITable *table);
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_simple_bb_merge_h_
