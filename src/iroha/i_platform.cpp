#include "iroha/i_platform.h"

#include "iroha/i_design.h"
#include "iroha/object_pool.h"
#include "iroha/resource_params.h"
#include "iroha/module_import.h"

namespace iroha {

IPlatform::IPlatform(IDesign *design) : design_(design), objects_(new ObjectPool) {
  if (design != nullptr) {
    design->GetObjectPool()->platforms_.Add(this);
  }
}

IPlatform::~IPlatform() {
}

ObjectPool *IPlatform::GetObjectPool() {
  return objects_;
}

namespace platform {

Condition::Condition(Definition *definition) : definition_(definition) {
  definition->GetPlatform()->GetObjectPool()->conditions_.Add(this);
}

Condition::~Condition() {
}

Definition *Condition::GetDefinition() {
  return definition_;
}

Value::Value(Definition *definition) : definition_(definition) {
  definition->GetPlatform()->GetObjectPool()->values_.Add(this);
}

Value::~Value() {
}

Definition *Value::GetDefinition() {
  return definition_;
}

Definition::Definition(IPlatform *platform)
  : platform_(platform), condition_(nullptr), value_(nullptr) {
}

Definition::~Definition() {
}

IPlatform *Definition::GetPlatform() {
  return platform_;
}

}  // namespace platform
}  // namespace iroha
