#include "opt/debug_annotation.h"

#include "iroha/i_design.h"
#include "writer/writer.h"

#include <fstream>

namespace iroha {
namespace opt {

DebugAnnotation::DebugAnnotation() : enabled_(false) {
}

DebugAnnotation::~DebugAnnotation() {
}

void DebugAnnotation::Enable() {
  enabled_ = true;
}

bool DebugAnnotation::IsEnabled() {
  return enabled_;
}

void DebugAnnotation::WriteToFiles(const string &fn) {
  ofstream os(fn);
  writer::Writer::WriteDumpHeader(os);
  os << dump_.str();
  writer::Writer::WriteDumpFooter(os);
}

void DebugAnnotation::DumpIntermediateTable(const ITable *tab) {
  writer::Writer::DumpTable(tab, dump_);
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

void DebugAnnotation::StartPhase(const string &name) {
  table_.clear();
  state_.clear();
}

}  // namespace opt
}  // namespace iroha
