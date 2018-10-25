#include "platform/platform_db.h"

#include "iroha/i_design.h"
#include "iroha/i_platform.h"

namespace iroha {
namespace platform {

PlatformDB::PlatformDB(IPlatform *platform) : platform_(platform) {
}

int PlatformDB::GetResourceDelay(IResource *res) {
  const string &klass = res->GetClass()->GetName();
  DefNode *value = FindValue("CLASS", klass);
  return GetInt(value, "DELAY", 0);
}

DefNode *PlatformDB::FindValue(const string &key, const string &value) {
  for (auto *def : platform_->defs_) {
    DefNode *cond = def->condition_;
    if (cond->GetHead() == key && cond->nodes_.size() == 2) {
      if (cond->nodes_[1]->str_ == value) {
	return def->value_;
      }
    }
  }
  return nullptr;
}

int PlatformDB::GetInt(DefNode *node, const string &key, int dflt) {
  if (node == nullptr) {
    return dflt;
  }
  for (DefNode *n : node->nodes_) {
    if (n->GetHead() == key && n->nodes_.size() >= 2) {
      DefNode *value = n->nodes_[1];
      if (value->is_atom_ && value->str_.empty()) {
	return value->num_;
      }
    }
  }
  return dflt;
}

}  // namespace platform
}  // namespace iroha
