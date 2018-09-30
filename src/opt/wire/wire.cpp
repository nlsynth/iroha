// WIP.
#include "opt/wire/wire.h"

#include "design/resource_attr.h"
#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "opt/bb_set.h"
#include "opt/debug_annotation.h"
#include "opt/delay_info.h"
#include "opt/wire/data_path.h"
#include "opt/wire/relocator.h"
#include "opt/wire/resource_share.h"
#include "opt/wire/scheduler.h"

namespace iroha {
namespace opt {
namespace wire {

Wire::Wire(ITable *table, DebugAnnotation *annotation)
  : table_(table), annotation_(annotation) {
  resource_share_.reset(new ResourceShare(table));
  data_path_set_.reset(new DataPathSet());
}

Wire::~Wire() {
}

bool Wire::Perform() {
  bset_.reset(BBSet::Create(table_, annotation_));
  if (annotation_->IsEnabled()) {
    annotation_->DumpIntermediateTable(table_);
  }

  resource_share_->Scan(bset_.get());
  resource_share_->Allocate();
  resource_share_->ReBind();

  // Assign ids to newly allocated insns.
  Validator::ValidateTable(table_);

  data_path_set_->Build(bset_.get());
  std::unique_ptr<DelayInfo> lat(DelayInfo::Create(10000));
  data_path_set_->SetDelay(lat.get());
  if (annotation_->IsEnabled()) {
    annotation_->StartSubSection("data_path", false);
    data_path_set_->Dump(annotation_);
    annotation_->ClearSubSection();
  }

  SchedulerCore sch(data_path_set_.get());
  sch.Schedule();

  Relocator rel(data_path_set_.get());
  rel.Relocate();

  return true;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
