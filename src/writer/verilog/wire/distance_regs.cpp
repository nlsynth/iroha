#include "writer/verilog/wire/distance_regs.h"

#include "writer/module_template.h"
#include "writer/verilog/resource.h"
#include "writer/verilog/table.h"
#include "writer/verilog/wire/accessor_info.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

DistanceRegs::DistanceRegs(AccessorInfo *ainfo, WireSet *wset)
  : ainfo_(ainfo), wset_(wset) {
}

void DistanceRegs::Build() {
  int distance = ainfo_->GetDistance();
  if (distance > 0) {
    BuildDistanceHandshake();
  } else {
    Build0DistanceHandshake();
  }
}

void DistanceRegs::Build0DistanceHandshake() {
  auto &tab = wset_->GetResource().GetTable();
  auto &asigs = ainfo_->GetSignals();
  ostream &rs = tab.ResourceSectionStream();
  ostream &rvs = tab.ResourceValueSectionStream();
  // accessor -> delay regs -> accessor edge -> resource.
  rvs << DEBUG_MESSAGE("  // DistanceRegs: begin");
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

void DistanceRegs::BuildDistanceHandshake() {
  // wire - wire_2of3 - wire_1of3 - wire0of3 - HS - edge_wire
  int distance = ainfo_->GetDistance();
  string dbg = "  // DistanceRegs: " + HandshakeStateReg() + " " +
    Util::Itoa(distance) + " begin";
  auto &tab = wset_->GetResource().GetTable();
  ostream &rs = tab.ResourceSectionStream();
  rs << "  reg [1:0] " << HandshakeStateReg() << ";\n";
  ostream &rvs = tab.ResourceValueSectionStream();
  rvs << DEBUG_MESSAGE(dbg.c_str());
  ostringstream reset_vals;
  ostringstream stage_vals;
  auto &asigs = ainfo_->GetSignals();
  for (auto *asig : asigs) {
    string wspec = Table::WidthSpec(asig->sig_desc_->width_);
    bool upstream = asig->sig_desc_->IsUpstream();
    string edge_wire = wset_->AccessorEdgeWireName(*asig);
    rs << "  wire " << wspec << edge_wire << ";\n";
    if (upstream) {
      string edge_src = EdgeSrcRegName(edge_wire);
      rs << "  reg " << wspec << edge_src << ";\n";
      reset_vals << "      " << edge_src << " <= 0;\n";
      rvs << "  assign " << edge_wire << " = " << edge_src << ";\n";
      rvs << "  assign " << StageRegName(*asig, distance - 1)
	  << " = " << wset_->AccessorWireName(*asig) << ";\n";
    } else {
      rvs << "  assign " << wset_->AccessorWireName(*asig) << " ="
	  << StageRegName(*asig, distance - 1) << ";\n";
    }
    for (int i = 0; i < distance; ++i) {
      bool is_last = (i == distance - 1);
      rs << "  ";
      if (upstream && is_last) {
	// Origin of upstream chain.
	rs << "wire";
      } else {
	reset_vals << "      " << StageRegName(*asig, i) << " <= 0;\n";
	rs << "reg";
      }
      rs << " " << wspec << StageRegName(*asig, i) << ";\n";
      // stage vals.
      if (upstream) {
	if (!is_last) {
	  stage_vals << "      " << StageRegName(*asig, i)
		     << " <= " << StageRegName(*asig, i + 1) << ";\n";
	}
      } else {
	if (i > 0) {
	  stage_vals << "      " << StageRegName(*asig, i)
		     << " <= " << StageRegName(*asig, i - 1) << ";\n";
	}
      }
    }
  }

  AccessorSignal *req_sig = nullptr;
  AccessorSignal *ack_sig = nullptr;
  for (auto *asig : asigs) {
    if (asig->sig_desc_->type_ == AccessorSignalType::ACCESSOR_REQ) {
      req_sig = asig;
    }
    if (asig->sig_desc_->type_ == AccessorSignalType::ACCESSOR_ACK) {
      ack_sig = asig;
    }
  }
  ostringstream hs;
  if (req_sig != nullptr && ack_sig != nullptr) {
    BuildHandshakeFSM(req_sig, ack_sig, hs, reset_vals);
  }

  tab.WriteAlwaysBlockHead(rvs);
  rvs << reset_vals.str();
  tab.WriteAlwaysBlockMiddle(rvs);
  rvs << stage_vals.str();
  rvs << hs.str();
  tab.WriteAlwaysBlockTail(rvs);
  rvs << DEBUG_MESSAGE("  // DistanceRegs end");
}

void DistanceRegs::BuildHandshakeFSM(AccessorSignal *req, AccessorSignal *ack,
				     ostream &os, ostream &is) {
  is << "      " << HandshakeStateReg() << " <= 0;\n";
  string req_i = StageRegName(*req, 0);
  string req_o = EdgeSrcRegName(wset_->AccessorEdgeWireName(*req));
  string ack_i = wset_->AccessorEdgeWireName(*ack);
  string ack_o = StageRegName(*ack, 0);
  string st = HandshakeStateReg();
  auto &asigs = ainfo_->GetSignals();
  for (auto *asig : asigs) {
    if (asig == req || asig == ack) {
      continue;
    }
    bool upstream = asig->sig_desc_->IsUpstream();
    if (upstream) {
      os << EdgeSrcRegName(wset_->AccessorEdgeWireName(*asig))
	 << " <= " << StageRegName(*asig, 0) << ";\n";
    } else {
      os << StageRegName(*asig, 0)
	 << " <= " << wset_->AccessorEdgeWireName(*asig) << ";\n";
    }
  }
  os << "      case (" << st << ")\n"
     << "        0: begin\n"
     << "          if (" << req_i << ") begin\n"
     << "            " << req_o << " <= 1;\n"
     << "            " << st << " <= 1;\n"
     << "          end\n"
     << "        end\n"
     << "        1: begin\n"
     << "          if (" << ack_i << ") begin\n"
     << "            " << req_o << " <= 0;\n"
     << "            " << ack_o << " <= 1;\n"
     << "            " << st << " <= 2;\n"
     << "          end\n"
     << "        end\n"
     << "        2: begin\n"
     << "          " << ack_o << " <= 0;\n"
     << "          " << st << " <= 3;\n"
     << "        end\n"
     << "        3: begin\n"
     << "          if (!" << req_i << ") begin\n"
     << "          " << st << " <= 0;\n"
     << "          end\n"
     << "        end\n"
     << "      endcase\n";
}

string DistanceRegs::StageRegName(const AccessorSignal &sig, int n) {
  int distance = ainfo_->GetDistance();
  return wset_->AccessorWireName(sig) + "_"
    + Util::Itoa(n) + "of" + Util::Itoa(distance);
}

string DistanceRegs::HandshakeStateReg() {
  return "st_" + Names::AccessorName(wset_->GetResourceName(),
				     ainfo_->GetResource());
}

string DistanceRegs::EdgeSrcRegName(const string &edge_wire) {
  return edge_wire + "_src";
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
