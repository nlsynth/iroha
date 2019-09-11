// -*- C++ -*-
#ifndef _writer_verilog_wire_wire_set_h_
#define _writer_verilog_wire_wire_set_h_

#include "writer/verilog/common.h"
// For convenience.
#include "writer/verilog/wire/names.h"

#include <map>

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class WireSet {
public:
  WireSet(Resource &res, const string &resource_name);
  ~WireSet();

  AccessorInfo *AddAccessor(const IResource *accessor);
  SignalDescription *GetSignalDescription(const string &name,
					  AccessorSignalType type, int width);
  string GetResourceName() const;
  Resource &GetResource() const;

  void Build();

  string AccessorWireName(const AccessorSignal &sig);
  string AccessorEdgeWireName(const AccessorSignal &sig);

private:
  void BuildAccessorWire(const SignalDescription &desc);
  void BuildResourceWire();
  void BuildDistanceRegs();
  void BuildArbitration(const SignalDescription &req_desc,
			const SignalDescription &ack_desc);
  void BuildRegisteredReq(const SignalDescription &req_desc,
			  vector<AccessorInfo *> &handshake_accessors);
  void BuildAccessorAck(const SignalDescription &rsig_desc,
			const SignalDescription &asig_desc,
			vector<AccessorInfo *> &handshake_accessors);
  void BuildWriteArg(const SignalDescription &arg_desc,
		     const SignalDescription *req_desc,
		     const SignalDescription *notify_desc,
		     const SignalDescription *notify_secondary_desc);
  void BuildReadArg(const SignalDescription &arg_desc);
  void BuildNotifyParent(const SignalDescription &desc);
  void BuildNotifyAccessor(const SignalDescription &desc);
  string ResourceWireName(const SignalDescription &desc);
  string AccessorWireNameWithReg(const AccessorSignal &sig);

  Resource &res_;
  string resource_name_;
  vector<AccessorInfo *> accessors_;
  map<string, SignalDescription *> signal_desc_;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_wire_set_h_
