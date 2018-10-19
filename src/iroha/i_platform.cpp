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
  delete objects_;
}

ObjectPool *IPlatform::GetObjectPool() {
  return objects_;
}

IDesign *IPlatform::GetDesign() {
  return design_;
}

void IPlatform::SetDesign(IDesign *design) {
  if (design_ == design) {
    return;
  }
  if (design_ != nullptr) {
    design_->GetObjectPool()->platforms_.Release(this);
  }
  design_ = design;
  if (design_ != nullptr) {
    design_->GetObjectPool()->platforms_.Add(this);
  }
}

namespace platform {

DefNode::DefNode(Definition *definition) : is_atom_(false), definition_(definition) {
  definition->GetPlatform()->GetObjectPool()->def_nodes_.Add(this);
}

DefNode::~DefNode() {
}

Definition *DefNode::GetDefinition() {
  return definition_;
}

const string &DefNode::GetHead() {
  static string empty_string;
  if (nodes_.size() == 0) {
    return empty_string;
  }
  return nodes_[0]->str_;
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
