// -*- C++ -*-
#ifndef _opt_debug_annotation_h_
#define _opt_debug_annotation_h_

#include "iroha/common.h"

#include <sstream>
#include <map>

namespace iroha {
namespace opt {

class DebugAnnotation {
public:
  ~DebugAnnotation();

  void DumpIntermediateTable(const ITable *tab);
  void GetDumpedContent(ostream &os);
  ostream &GetDumpStream();

  ostream &Table(const ITable *tab);
  string GetTableAnnotation(const ITable *tab) const;

  ostream &State(const IState *st);
  string GetStateAnnotation(const IState *st) const;

private:
  ostringstream dump_;

  map<const ITable *, ostringstream> table_;
  map<const IState *, ostringstream> state_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_debug_annotation_h_
