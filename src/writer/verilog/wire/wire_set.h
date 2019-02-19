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

enum AccessorSignalType : int {
  ACCESSOR_REQ,
  ACCESSOR_ACK,
  ACCESSOR_READ_ARG,
  ACCESSOR_WRITE_ARG,
  ACCESSOR_NOTIFY_PARENT,
  ACCESSOR_NOTIFY_PARENT_SECONDARY,
  ACCESSOR_NOTIFY_ACCESSOR,
};

class SignalDescription {
public:
  // e.g. ack, req...
  string name_;
  AccessorSignalType type_;
  int width_;
};

class AccessorInfo;

// Per accessor and signal.
class AccessorSignal {
public:
  SignalDescription *sig_desc_;
  const IResource *accessor_res_;
  AccessorInfo *accessor_info_;
};

class WireSet;

class AccessorInfo {
public:
  AccessorInfo(WireSet *wire_set, const IResource *accessor);
  ~AccessorInfo();

  void AddSignal(const string &name, AccessorSignalType type, int width);
  const vector<AccessorSignal *> &GetSignals();
  AccessorSignal *FindSignal(const SignalDescription &desc);
  void SetDistance(int distance);
  int GetDistance() const;

private:
  WireSet *wire_set_;
  const IResource *accessor_;
  string accessor_name_;
  vector<AccessorSignal *> accessor_signals_;
  int distance_;
};

class WireSet {
public:
  WireSet(Resource &res, const string &resource_name);
  ~WireSet();

  AccessorInfo *AddAccessor(const IResource *accessor);
  SignalDescription *GetSignalDescription(const string &name,
					  AccessorSignalType type, int width);
  string GetResourceName() const;

  void Build();

private:
  void BuildAccessorWire(const SignalDescription &desc);
  void BuildResourceWire();
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
  string AccessorWireName(const AccessorSignal &sig);
  string AccessorEdgeWireName(const AccessorSignal &sig);
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
