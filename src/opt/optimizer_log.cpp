#include "opt/optimizer_log.h"

#include <fstream>

#include "iroha/i_design.h"
#include "iroha/stl_util.h"
#include "writer/writer.h"

namespace iroha {
namespace opt {

class OptimizerLogSection {
 public:
  OptimizerLogSection() : is_html_(false){};

  string dumpFileName(const string &base, const string &suffix) {
    string fn = base + "." + suffix;
    if (is_html_) {
      fn = fn + ".html";
    }
    return fn;
  }

  ostringstream dump_;
  bool is_html_;
};

OptimizerLog::OptimizerLog() : enabled_(false), current_pass_index_(0) {}

OptimizerLog::~OptimizerLog() { STLDeleteSecondElements(&sections_); }

void OptimizerLog::Enable() { enabled_ = true; }

bool OptimizerLog::IsEnabled() { return enabled_; }

void OptimizerLog::WriteToFiles(const string &baseFn) {
  for (auto &p : sections_) {
    OptimizerLogSection *ls = p.second;
    string fn = ls->dumpFileName(baseFn, p.first);
    ofstream os(fn);
    if (ls->is_html_) {
      writer::Writer::WriteDumpHeader(os);
    }
    os << ls->dump_.str();
    if (ls->is_html_) {
      writer::Writer::WriteDumpFooter(os);
    }
  }
  // Index of generated dump files.
  ofstream os(baseFn + ".html");
  writer::Writer::WriteDumpHeader(os);
  for (auto &p : sections_) {
    OptimizerLogSection *ls = p.second;
    string linkFn = ls->dumpFileName(baseFn, p.first);
    linkFn = Util::BaseName(linkFn);
    os << "<a href=\"" << linkFn << "\">" << linkFn << "</a>\n";
  }
  writer::Writer::WriteDumpFooter(os);
}

void OptimizerLog::DumpTable(const ITable *tab) {
  writer::Writer::DumpTable(tab, GetSection(current_file_name_)->dump_);
}

ostream &OptimizerLog::GetDumpStream() {
  return GetSection(current_file_name_)->dump_;
}

ostream &OptimizerLog::Table(const ITable *tab) { return table_[tab]; }

string OptimizerLog::GetTableAnnotation(const ITable *tab) const {
  auto it = table_.find(tab);
  if (it == table_.end()) {
    return string();
  }
  return it->second.str();
}

ostream &OptimizerLog::State(const IState *st) { return state_[st]; }

void OptimizerLog::SetStateColorIndex(const IState *st, int idx) {
  state_color_[st] = idx;
}

string OptimizerLog::GetStateAnnotation(const IState *st) const {
  auto it = state_.find(st);
  if (it == state_.end()) {
    return string();
  }
  return it->second.str();
}

int OptimizerLog::GetStateColorIndex(const IState *st) const {
  auto p = state_color_.find(st);
  if (p == state_color_.end()) {
    return 0;
  }
  return p->second;
}

void OptimizerLog::StartPass(const string &name) {
  string s = Util::Itoa(current_pass_index_);
  while (s.length() < 3) {
    s = "0" + s;
  }
  current_pass_name_ = s + name;
  current_pass_index_++;
  current_section_name_ = "";
  UpdateFileName(true);
  table_.clear();
  state_.clear();
}

void OptimizerLog::StartSubSection(const string &section, bool isHtml) {
  current_section_name_ = section;
  UpdateFileName(isHtml);
}

void OptimizerLog::ClearSubSection() {
  current_section_name_.clear();
  UpdateFileName(false);
}

void OptimizerLog::UpdateFileName(bool isHtml) {
  current_file_name_ = current_pass_name_;
  if (!current_section_name_.empty()) {
    current_file_name_ += "." + current_section_name_;
  }
  OptimizerLogSection *ls = GetSection(current_file_name_);
  ls->is_html_ = isHtml;
}

OptimizerLogSection *OptimizerLog::GetSection(const string &name) {
  auto it = sections_.find(name);
  if (it == sections_.end()) {
    OptimizerLogSection *s = new OptimizerLogSection();
    sections_[name] = s;
    return s;
  }
  return it->second;
}

}  // namespace opt
}  // namespace iroha
