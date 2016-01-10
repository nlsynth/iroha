#include "opt/bb_set.h"

#include "opt/bb_collector.h"

namespace iroha {
namespace opt {

BBSet *BBSet::Create(ITable *table) {
  BBCollector collector(table);
  return collector.Create();
}

}  // namespace opt
}  // namespace iroha
