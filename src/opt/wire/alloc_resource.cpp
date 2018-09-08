// WIP. This inherits implementation from wire_insn phase.
#include "opt/wire/alloc_resource.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace opt {
namespace wire {

AllocResourcePhase::~AllocResourcePhase() {
}

Phase *AllocResourcePhase::Create() {
  return new AllocResourcePhase();
}

bool AllocResourcePhase::ApplyForTable(const string &key, ITable *table) {
  AllocResource alloc_resource(table, annotation_);
  return alloc_resource.Perform();
}

AllocResource::AllocResource(ITable *table, DebugAnnotation *annotation)
  : Scaffold(table, annotation) {
}

AllocResource::~AllocResource() {
}

bool AllocResource::Perform() {
  return true;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
