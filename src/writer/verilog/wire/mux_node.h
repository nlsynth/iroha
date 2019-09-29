// -*- C++ -*-
#ifndef _writer_verilog_wire_mux_node_h_
#define _writer_verilog_wire_mux_node_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class MuxNode {
public:
  MuxNode(const Mux *mux, int id, const AccessorInfo *accessor);

  void WriteIOWire(Ports *ports, ostream &os);
  void WriteMux(ostream &os);
  bool IsRoot() const;
  bool IsLeaf() const;

  vector<MuxNode *> children_;

private:
  const Mux *mux_;
  int id_;
  const WireSet *ws_;
  const AccessorInfo *accessor_;

  void BuildArbitration(const SignalDescription &req_desc,
			const SignalDescription &ack_desc,
			ostream &os);
  void BuildWriteArg(const SignalDescription &arg_desc,
		     const SignalDescription *req_desc,
		     const SignalDescription *notify_desc,
		     const SignalDescription *notify_secondary_desc,
		     ostream &os);
  void BuildReadArg(const SignalDescription &arg_desc,
		    ostream &os);
  void BuildNotifyParent(const SignalDescription &desc, ostream &os);
  void BuildNotifyAccessor(const SignalDescription &desc, ostream &os);
  void BuildRegisteredReq(const SignalDescription &req_desc,
			  vector<const AccessorInfo *> &handshake_accessors,
			  ostream &os);
  void BuildAccessorAck(const SignalDescription &rsig_desc,
			const SignalDescription &asig_desc,
			vector<const AccessorInfo *> &handshake_accessors,
			ostream &os);

  string AccessorEdgeWireName(const AccessorSignal &sig) const;
  string AccessorWireNameWithReg(const AccessorSignal &sig) const;
  string ResourceWireName(const SignalDescription &desc) const;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_node_h_
