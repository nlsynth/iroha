#include "opt/bb_set.h"

#include "opt/bb_collector.h"
#include "iroha/stl_util.h"

namespace iroha {
namespace opt {

BBSet::~BBSet() {
  STLDeleteValues(&bbs_);
}

BBSet *BBSet::Create(ITable *table) {
  BBCollector collector(table);
  return collector.Create();
}

}  // namespace opt
}  // namespace iroha
