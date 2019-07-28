#include "iroha/i_platform.h"

#include "iroha/i_design.h"
#include "iroha/object_pool.h"
#include "iroha/resource_params.h"

namespace iroha {

IPlatform::IPlatform(IDesign *design)
  : design_(design), objects_(new ObjectPool) {
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

const string &IPlatform::GetName() const {
  return name_;
}

void IPlatform::SetName(const string &name) {
  name_ = name;
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
  : condition_(nullptr), value_(nullptr), platform_(platform) {
}

Definition::~Definition() {
}

IPlatform *Definition::GetPlatform() {
  return platform_;
}

}  // namespace platform
}  // namespace iroha
