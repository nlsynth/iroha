#include "opt/bb_shrink.h"

#include "opt/bb_set.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

BBShrinkPhase::~BBShrinkPhase() {
}

Phase *BBShrinkPhase::Create() {
  return new BBShrinkPhase();
}

bool BBShrinkPhase::ApplyForTable(ITable *table) {
  unique_ptr<BBSet> bbs(BBSet::Create(table, annotation_));
  for (BB *bb : bbs->bbs_) {
    ShrinkBB(bb);
  }
  return true;
}

void BBShrinkPhase::ShrinkBB(BB *bb) {
}

}  // namespace opt
}  // namespace iroha
