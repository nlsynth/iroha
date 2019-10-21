// -*- C++ -*-
#ifndef _writer_verilog_wire_mux_node_h_
#define _writer_verilog_wire_mux_node_h_

#include "writer/verilog/common.h"

#include <sstream>

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class MuxNode {
public:
  MuxNode(const Mux *mux, int id, const AccessorInfo *accessor);

  void WriteIOWire(Ports *ports, ostream &os);
  void WriteDecls(ostream &os);
  void WriteMux(ostream &os);
  bool IsRoot() const;
  bool IsLeaf() const;

  string NodeWireName(const SignalDescription &desc) const;

  vector<MuxNode *> children_;

private:
  const Mux *mux_;
  int id_;
  const WireSet *ws_;
  const AccessorInfo *accessor_;
  // initial registers values for staged mux.
  ostringstream is_;
  // stage updates for staged mux.
  ostringstream ss_;
  // wire assigns for staged mux.
  ostringstream as_;
  // wdata capture for staged mux.
  ostringstream cs_;
  // default control value for staged mux.
  ostringstream ds_;

  bool IsStaged() const;
  void BuildArbitration(const SignalDescription &req_desc,
			const SignalDescription &ack_desc,
			ostream &os);
  void BuildWriteArg(const SignalDescription &arg_desc,
		     const SignalDescription *req_desc,
		     const SignalDescription *ack_desc,
		     const SignalDescription *notify_desc,
		     const SignalDescription *notify_secondary_desc,
		     ostream &os);
  void BuildReadArg(const SignalDescription &arg_desc,
		    ostream &os);
  void BuildNotifyParent(const SignalDescription &desc, ostream &os);
  void BuildNotifyAccessor(const SignalDescription &desc, ostream &os);
  void BuildReqState(const SignalDescription &req_desc,
		     const SignalDescription &ack_desc,
		     vector<const MuxNode *> &handshake_nodes,
		     ostream &os);
  void BuildAccessorAck(const SignalDescription &rsig_desc,
			const SignalDescription &asig_desc,
			vector<const MuxNode *> &handshake_nodes,
			ostream &os);
  void BuildStageHandShake(const SignalDescription &req_desc,
			   const SignalDescription &ack_desc);
  void WriteStage(SignalDescription *req, SignalDescription *ack,
		  SignalDescription *notify_secondary, ostream &os);

  string NodeWireNameWithReg(const SignalDescription &desc) const;
  string NodeWireNameWithSrc(const SignalDescription &desc) const;
  string NodeWireNameWithPrev(const SignalDescription &desc) const;
  string HandShakeState() const;
  string ReqState() const;
  string ReqStateWire() const;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_node_h_
