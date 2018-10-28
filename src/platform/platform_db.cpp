#include "platform/platform_db.h"

#include "iroha/i_design.h"
#include "iroha/i_platform.h"

namespace iroha {
namespace platform {

class LookupCondition {
public:
  string klass;
};

PlatformDB::PlatformDB(IPlatform *platform) : platform_(platform) {
}

int PlatformDB::GetResourceDelay(IResource *res) {
  const string &klass = res->GetClass()->GetName();
  LookupCondition cond;
  cond.klass = klass;
  DefNode *value = FindValue(cond);
  return GetInt(value, "DELAY", 0);
}

DefNode *PlatformDB::FindValue(const LookupCondition &lookup_cond) {
  for (auto *def : platform_->defs_) {
    DefNode *cond = def->condition_;
    if (MatchCond(lookup_cond, cond)) {
      return def->value_;
    }
  }
  return nullptr;
}

bool PlatformDB::MatchCond(const LookupCondition &lookup_cond, DefNode *cond_node) {
  const string &head = cond_node->GetHead();
  if (head == "CLASS" && cond_node->nodes_.size() == 2) {
    if (cond_node->nodes_[1]->str_ == lookup_cond.klass) {
      return true;
    }
  }
  return false;
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
