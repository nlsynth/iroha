// -*- C++ -*-
#ifndef _writer_verilog_wire_distance_regs_h_
#define _writer_verilog_wire_distance_regs_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class AccessorInfo;

class DistanceRegs {
public:
  DistanceRegs(AccessorInfo *ainfo, WireSet *wset);

  void Build();

private:
  AccessorInfo *ainfo_;
  WireSet *wset_;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_distance_regs_h_
