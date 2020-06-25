#include "opt/simple_bb_merge.h"

namespace iroha {
namespace opt {

SimpleBBMerge::~SimpleBBMerge() {
}

Phase *SimpleBBMerge::Create() {
  return new SimpleBBMerge();
}

bool SimpleBBMerge::ApplyForTable(const string &key, ITable *table) {
  return true;
}

}  // namespace opt
}  // namespace iroha
