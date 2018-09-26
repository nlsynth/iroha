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

void DebugAnnotation::WriteToFiles(const string &baseFn) {
  for (auto &p : dump_) {
    const string &suffix = p.first;
    string fn = baseFn + "." + suffix;
    bool isHtml = (html_sections_.find(suffix) != html_sections_.end());
    if (isHtml) {
      fn += ".html";
    }
    ofstream os(fn);
    if (isHtml) {
      writer::Writer::WriteDumpHeader(os);
    }
    os << p.second.str();
    if (isHtml) {
      writer::Writer::WriteDumpFooter(os);
    }
  }
  // Index of generated dump files.
  ofstream os(baseFn + ".html");
  writer::Writer::WriteDumpHeader(os);
  for (auto &p : dump_) {
    // TODO: Use basename.
    const string &suffix = p.first;
    string linkFn = baseFn + "." + suffix;
    bool isHtml = (html_sections_.find(suffix) != html_sections_.end());
    if (isHtml) {
      linkFn += ".html";
    }
    linkFn = Util::BaseName(linkFn);
    os << "<a href=\"" << linkFn << "\">" << linkFn << "</a>\n";
  }
  writer::Writer::WriteDumpFooter(os);
}

void DebugAnnotation::DumpIntermediateTable(const ITable *tab) {
  writer::Writer::DumpTable(tab, dump_[file_name_]);
}

ostream &DebugAnnotation::GetDumpStream() {
  return dump_[file_name_];
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

void DebugAnnotation::SetStateColorIndex(const IState *st, int idx) {
  state_color_[st] = idx;
}

string DebugAnnotation::GetStateAnnotation(const IState *st) const {
  auto it = state_.find(st);
  if (it == state_.end()) {
    return string();
  }
  return it->second.str();
}

int DebugAnnotation::GetStateColorIndex(const IState *st) const {
  auto p = state_color_.find(st);
  if (p == state_color_.end()) {
    return 0;
  }
  return p->second;
}

void DebugAnnotation::StartPhase(const string &name) {
  phase_name_ = name;
  section_name_ = "";
  UpdateFileName(true);
  table_.clear();
  state_.clear();
}

void DebugAnnotation::StartSubSection(const string &section, bool isHtml) {
  section_name_ = section;
  UpdateFileName(isHtml);
}

void DebugAnnotation::ClearSubSection() {
  section_name_.clear();
  UpdateFileName(false);
}

void DebugAnnotation::UpdateFileName(bool isHtml) {
  file_name_ = phase_name_;
  if (!section_name_.empty()) {
    file_name_ += "." + section_name_;
  }
  if (isHtml) {
    html_sections_.insert(file_name_);
  }
}

}  // namespace opt
}  // namespace iroha
