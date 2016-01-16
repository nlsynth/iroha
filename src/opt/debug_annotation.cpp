#include "opt/debug_annotation.h"

namespace iroha {
namespace opt {

DebugAnnotation::~DebugAnnotation() {
}

ostream &DebugAnnotation::Table(const ITable *tab) {
  return table_[tab];
}

string DebugAnnotation::GetTableAnnotation(const ITable *tab) const {
  auto it = table_.find(tab);
  if (it == table_.end()) {
    return string();
  }
  return it->second.str();
}

ostream &DebugAnnotation::State(const IState *st) {
  return state_[st];
}

string DebugAnnotation::GetStateAnnotation(const IState *st) const {
  auto it = state_.find(st);
  if (it == state_.end()) {
    return string();
  }
  return it->second.str();
}

}  // namespace opt
}  // namespace iroha
