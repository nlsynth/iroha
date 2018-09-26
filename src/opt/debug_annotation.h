// -*- C++ -*-
#ifndef _opt_debug_annotation_h_
#define _opt_debug_annotation_h_

#include "iroha/common.h"

#include <sstream>
#include <map>
#include <set>

namespace iroha {
namespace opt {

// Per design object to store debug strings.
class DebugAnnotation {
public:
  DebugAnnotation();
  ~DebugAnnotation();

  // Called once at the start.
  void Enable();
  // Called at the end.
  void WriteToFiles(const string &baseFn);
  // Called at beginning of each phase.
  void StartPhase(const string &name);
  // Dump into a separate file from the main output from the phase.
  void StartSubSection(const string &section, bool isHtml);
  void ClearSubSection();
  // Called by optimizers.
  bool IsEnabled();
  void DumpIntermediateTable(const ITable *tab);
  ostream &GetDumpStream();
  ostream &Table(const ITable *tab);
  ostream &State(const IState *st);
  void SetStateColorIndex(const IState *st, int idx);

  // Called by HTML dumper.
  string GetTableAnnotation(const ITable *tab) const;
  string GetStateAnnotation(const IState *st) const;
  int GetStateColorIndex(const IState *st) const;

private:
  void UpdateFileName(bool isHtml);

  bool enabled_;
  string phase_name_;
  string section_name_;
  string file_name_;
  map<string, ostringstream> dump_;
  set<string> html_sections_;

  map<const ITable *, ostringstream> table_;
  map<const IState *, ostringstream> state_;
  map<const IState *, int> state_color_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_debug_annotation_h_
