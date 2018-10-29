#include "platform/platform_db.h"

#include "iroha/i_design.h"
#include "iroha/i_platform.h"

namespace iroha {
namespace platform {

class LookupCondition {
public:
  string klass;
};

NodeResult::NodeResult(bool b) : is_bool_(true), bv_(b), iv_(0) {
}

NodeResult::NodeResult(int iv) : is_bool_(false), bv_(false), iv_(iv) {
}

bool NodeResult::IsBool() const {
  return is_bool_;
}

bool NodeResult::BoolVal() const {
  return bv_;
}

int NodeResult::IntVal() const {
  return iv_;
}

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
    NodeResult nr = MatchCond(lookup_cond, cond);
    if (nr.BoolVal()) {
      return def->value_;
    }
  }
  return nullptr;
}

NodeResult PlatformDB::MatchCond(const LookupCondition &lookup_cond, DefNode *cond_node) {
  const string &head = cond_node->GetHead();
  if (head == "CLASS" && cond_node->nodes_.size() == 2) {
    if (cond_node->nodes_[1]->str_ == lookup_cond.klass) {
      return NodeResult(true);
    }
  }
  if (head == "AND") {
    for (int i = 1; i < cond_node->nodes_.size(); ++i) {
      NodeResult nr = MatchCond(lookup_cond, cond_node->nodes_[i]);
      if (!nr.BoolVal()) {
	return NodeResult(false);
      }
    }
    return NodeResult(true);
  }
  return NodeResult(false);
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
