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
  ~DebugAnnotation();

  // Called by optimizers.
  void DumpIntermediateTable(const ITable *tab);
  void GetDumpedContent(ostream &os);
  ostream &GetDumpStream();
  ostream &Table(const ITable *tab);
  ostream &State(const IState *st);
  void Clear();
  void ClearForTable(const ITable *tab);

  // Called by HTML dumper.
  string GetTableAnnotation(const ITable *tab) const;
  string GetStateAnnotation(const IState *st) const;

private:
  ostringstream dump_;

  map<const ITable *, ostringstream> table_;
  map<const IState *, ostringstream> state_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_debug_annotation_h_
