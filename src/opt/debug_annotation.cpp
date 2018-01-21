#include "opt/debug_annotation.h"

#include "iroha/i_design.h"
#include "writer/writer.h"

namespace iroha {
namespace opt {

DebugAnnotation::~DebugAnnotation() {
}

void DebugAnnotation::DumpIntermediateTable(const ITable *tab) {
  writer::Writer::DumpTable(tab, dump_);
}

void DebugAnnotation::GetDumpedContent(ostream &os) {
  os << dump_.str();
}

ostream &DebugAnnotation::GetDumpStream() {
  return dump_;
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

void DebugAnnotation::ClearForTable(const ITable *tab) {
  table_.erase(tab);
  for (auto it = state_.begin(); it != state_.end(); ) {
    const auto *st = it->first;
    if (st->GetTable() == tab) {
      it = state_.erase(it);
    } else {
      ++it;
    }
  }
}

void DebugAnnotation::Clear() {
  table_.clear();
  state_.clear();
}

}  // namespace opt
}  // namespace iroha
