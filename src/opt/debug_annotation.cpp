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
  for (auto &p : dump_) {
    ofstream os(fn + "." + p.first);
    writer::Writer::WriteDumpHeader(os);
    os << p.second.str();
    writer::Writer::WriteDumpFooter(os);
  }
  // Index of generated dump files.
  ofstream os(fn);
  writer::Writer::WriteDumpHeader(os);
  for (auto &p : dump_) {
    // TODO: Use basename.
    string f = fn + "." + p.first;
    os << "<a href=\"" << f << "\">" << f << "</a>\n";
  }
  writer::Writer::WriteDumpFooter(os);
}

void DebugAnnotation::DumpIntermediateTable(const ITable *tab) {
  writer::Writer::DumpTable(tab, dump_[phase_name_]);
}

ostream &DebugAnnotation::GetDumpStream() {
  return dump_[phase_name_];
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
  phase_name_ = name;
  table_.clear();
  state_.clear();
}

}  // namespace opt
}  // namespace iroha
