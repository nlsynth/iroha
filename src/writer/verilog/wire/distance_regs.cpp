#include "writer/verilog/wire/distance_regs.h"

#include "writer/module_template.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

DistanceRegs::DistanceRegs(AccessorInfo *ainfo, WireSet *wset)
  : ainfo_(ainfo), wset_(wset) {
}

void DistanceRegs::Build() {
  auto &tab = wset_->GetResource().GetTable();
  auto &asigs = ainfo_->GetSignals();
  ostream &rs = tab.ResourceSectionStream();
  ostream &rvs = tab.ResourceValueSectionStream();
  string dbg = "  // DistanceRegs: begin";
  // accessor -> delay regs (TODO) -> accessor edge -> resource.
  rvs << DEBUG_MESSAGE(dbg.c_str());
  for (auto &asig : asigs) {
    bool upstream = asig->sig_desc_->IsUpstream();
    string lhs, rhs;
    if (upstream) {
      lhs = wset_->AccessorEdgeWireName(*asig);
      rhs = wset_->AccessorWireName(*asig);
    } else {
      lhs = wset_->AccessorWireName(*asig);
      rhs = wset_->AccessorEdgeWireName(*asig);
    }
    rvs << "  assign " << lhs << " = " << rhs << ";\n";
    rs << "  wire " << Table::WidthSpec(asig->sig_desc_->width_)
       << wset_->AccessorEdgeWireName(*asig) << ";\n";
  }
  rvs << DEBUG_MESSAGE("  // DistanceRegs end");
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
