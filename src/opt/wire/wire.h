// -*- C++ -*-
#ifndef _opt_wire_wire_h_
#define _opt_wire_wire_h_

#include "opt/wire/scaffold.h"

namespace iroha {
namespace opt {
namespace wire {

class Wire : public Scaffold {
public:
  Wire(ITable *table, DebugAnnotation *annotation);
  virtual ~Wire();
  bool Perform();

private:
  unique_ptr<ResourceShare> resource_share_;
};

}  // namespace wire
}  // namespace opt
}  // namespace iroha

#endif  // _opt_wire_wire_h_
