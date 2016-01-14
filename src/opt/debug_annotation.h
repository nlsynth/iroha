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

  ostream &State(const IState *st);
  string GetStateAnnotation(const IState *st) const;

private:
  map<const IState *, ostringstream> state_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_debug_annotation_h_
