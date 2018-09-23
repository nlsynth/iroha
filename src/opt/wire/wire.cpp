// WIP.
#include "opt/wire/wire.h"

#include "design/resource_attr.h"
#include "design/validator.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "opt/debug_annotation.h"
#include "opt/latency_info.h"
#include "opt/wire/data_path.h"
#include "opt/wire/resource_share.h"

namespace iroha {
namespace opt {
namespace wire {

Wire::Wire(ITable *table, DebugAnnotation *annotation)
  : Scaffold(table, annotation) {
  resource_share_.reset(new ResourceShare(table));
  data_path_set_.reset(new DataPathSet());
}

Wire::~Wire() {
}

bool Wire::Perform() {
  SetUp();
  resource_share_->Scan(bset_.get());
  resource_share_->Allocate();
  resource_share_->ReBind();

  // Assign ids to newly allocated insns.
  Validator::ValidateTable(table_);

  data_path_set_->Build(bset_.get());
  LatencyInfo lat;
  data_path_set_->SetLatency(&lat);
  if (annotation_->IsEnabled()) {
    annotation_->StartSubSection("data_path", false);
    data_path_set_->Dump(annotation_);
    annotation_->ClearSubSection();
  }

  return true;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
