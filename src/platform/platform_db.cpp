#include "platform/platform_db.h"

#include "iroha/i_design.h"
#include "iroha/i_platform.h"

namespace iroha {
namespace platform {

class LookupCondition {
public:
  string klass;
  vector<int> inputs_;
};

NodeResult::NodeResult()
  : has_error_(false), is_bool_(true), bv_(false), iv_(0) {
}

NodeResult::NodeResult(bool b)
  : has_error_(false), is_bool_(true), bv_(b), iv_(0) {
}

NodeResult::NodeResult(int iv)
  : has_error_(false), is_bool_(false), bv_(false), iv_(iv) {
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

bool NodeResult::HasError() const {
  return has_error_;
}

NodeResult NodeResult::SetError() {
  has_error_ = true;
  return *this;
}

NodeResult NodeResult::ErrorNode() {
  return NodeResult().SetError();
}

PlatformDB::PlatformDB(IPlatform *platform) : platform_(platform) {
}

int PlatformDB::GetResourceDelay(IResource *res) {
  const string &klass = res->GetClass()->GetName();
  LookupCondition cond;
  cond.klass = klass;
  for (auto &ivt : res->input_types_) {
    cond.inputs_.push_back(ivt.GetWidth());
  }
  DefNode *value = FindValue(cond);
  return GetInt(value, "DELAY", 0);
}

DefNode *PlatformDB::FindValue(const LookupCondition &lookup_cond) {
  for (auto *def : platform_->defs_) {
    DefNode *cond = def->condition_;
    NodeResult nr = EvalNode(lookup_cond, cond);
    if (nr.HasError()) {
      // Skip errorneous condition for now.
      continue;
    }
    if (nr.BoolVal()) {
      return def->value_;
    }
  }
  return nullptr;
}

NodeResult PlatformDB::EvalNode(const LookupCondition &lookup_cond, DefNode *cond_node) {
  if (cond_node->is_atom_) {
    return NodeResult(cond_node->num_);
  }
  const string &head = cond_node->GetHead();
  if (head == "CLASS") {
    if (cond_node->nodes_.size() != 2) {
      return NodeResult::ErrorNode();
    }
    if (cond_node->nodes_[1]->str_ == lookup_cond.klass) {
      return NodeResult(true);
    }
  }
  if (head == "AND") {
    for (int i = 1; i < cond_node->nodes_.size(); ++i) {
      NodeResult nr = EvalNode(lookup_cond, cond_node->nodes_[i]);
      if (nr.HasError() || !nr.IsBool()) {
	return NodeResult::ErrorNode();
      }
      if (!nr.BoolVal()) {
	return NodeResult(false);
      }
    }
    return NodeResult(true);
  }
  if (head == "INPUT") {
    if (cond_node->nodes_.size() != 2) {
      return NodeResult::ErrorNode();
    }
    int idx = cond_node->nodes_[1]->num_;
    if (idx < lookup_cond.inputs_.size()) {
      return NodeResult(lookup_cond.inputs_[idx]);
    } else {
      return NodeResult::ErrorNode();
    }
  }
  if (head == ">") {
    return EvalCompare(lookup_cond, cond_node, true);
  }
  if (head == "<") {
    return EvalCompare(lookup_cond, cond_node, false);
  }
  return NodeResult(false);
}

NodeResult PlatformDB::EvalCompare(const LookupCondition &lookup_cond, DefNode *cond_node, bool isGt) {
  if (cond_node->nodes_.size() != 3) {
    return NodeResult::ErrorNode();
  }
  NodeResult lhs = EvalNode(lookup_cond, cond_node->nodes_[1]);
  NodeResult rhs = EvalNode(lookup_cond, cond_node->nodes_[2]);
  if (lhs.HasError() || lhs.IsBool()) {
    return NodeResult::ErrorNode();
  }
  if (rhs.HasError() || rhs.IsBool()) {
    return NodeResult::ErrorNode();
  }
  int lhs_val = lhs.IntVal();
  int rhs_val = rhs.IntVal();
  if (lhs_val == rhs_val) {
    return NodeResult(false);
  }
  bool gt = lhs_val > rhs_val;
  return NodeResult(isGt ? gt : !gt);
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
