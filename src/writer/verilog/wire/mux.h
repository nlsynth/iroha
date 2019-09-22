// -*- C++ -*-
#ifndef _writer_verilog_wire_mux_h_
#define _writer_verilog_wire_mux_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

// WIP: This will host a tree, but only 1 level for now.
struct MuxNode {
  MuxNode();

  const AccessorInfo *accessor_;
  vector<MuxNode *> children_;
};

class Mux {
public:
  Mux(const WireSet *ws);
  ~Mux();

  static void Write(const WireSet *ws, ostream &os);

  void DoWrite(ostream &os);

private:
  const WireSet *ws_;
  unique_ptr<Ports> ports_;
  MuxNode *root_node_;

  void WriteMux(ostream &os);
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
  MuxNode *BuildNodes(const vector<AccessorInfo *> &acs);
  void WriteIOWire(MuxNode *node);
  void DeleteNode(MuxNode *node);
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_mux_h_
