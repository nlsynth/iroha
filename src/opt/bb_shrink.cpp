#include "opt/bb_shrink.h"

#include "opt/bb_set.h"
#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

BBShrink::~BBShrink() {
}

Phase *BBShrink::Create() {
  return new BBShrink();
}

bool BBShrink::ApplyForTable(ITable *table) {
  unique_ptr<BBSet> bbs(BBSet::Create(table, annotation_));
  for (BB *bb : bbs->bbs_) {
    ShrinkBB(bb);
  }
  return true;
}

void BBShrink::ShrinkBB(BB *bb) {
}

}  // namespace opt
}  // namespace iroha
