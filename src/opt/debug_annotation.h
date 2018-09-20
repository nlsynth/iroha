// -*- C++ -*-
#ifndef _opt_debug_annotation_h_
#define _opt_debug_annotation_h_

#include "iroha/common.h"

#include <sstream>
#include <map>

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
  void WriteToFiles(const string &fn);
  // Called at beginning of each phase.
  void StartPhase(const string &name);
  // Called by optimizers.
  bool IsEnabled();
  void DumpIntermediateTable(const ITable *tab);
  ostream &GetDumpStream();
  ostream &Table(const ITable *tab);
  ostream &State(const IState *st);

  // Called by HTML dumper.
  string GetTableAnnotation(const ITable *tab) const;
  string GetStateAnnotation(const IState *st) const;

private:
  bool enabled_;
  string phase_name_;
  map<string, ostringstream> dump_;

  map<const ITable *, ostringstream> table_;
  map<const IState *, ostringstream> state_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_debug_annotation_h_
