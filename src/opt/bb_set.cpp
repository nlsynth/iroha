#include "opt/bb_set.h"

#include "opt/bb_collector.h"
#include "opt/debug_annotation.h"
#include "iroha/stl_util.h"

namespace iroha {
namespace opt {

BBSet::~BBSet() {
  STLDeleteValues(&bbs_);
}

BBSet *BBSet::Create(ITable *table, DebugAnnotation *annotation) {
  BBCollector collector(table, annotation);
  return collector.Create();
}

void BBSet::Annotate(DebugAnnotation *annotation) {
  for (int i = 0; i < bbs_.size(); ++i) {
    for (IState *st : bbs_[i]->states_) {
      annotation->State(st) << "bb:" << i;
    }
  }
}

}  // namespace opt
}  // namespace iroha
