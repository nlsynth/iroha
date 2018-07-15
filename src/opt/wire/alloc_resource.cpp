// WIP. This inherits implementation from wire_insn phase.
#include "opt/wire/alloc_resource.h"

#include "iroha/i_design.h"

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
  : WireInsn(table, annotation) {
}

AllocResource::~AllocResource() {
}

bool AllocResource::CanUseResourceInState(IState *st, IResource *resource) {
  // TODO: Implementation for this class.
  return WireInsn::CanUseResourceInState(st, resource);
}

IInsn *AllocResource::MayCopyInsnForState(IState *st, IInsn *insn) {
  // TODO: Implementation for this class.
  return insn;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
