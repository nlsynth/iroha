// -*- C++ -*-
#ifndef _writer_verilog_wire_wire_set_h_
#define _writer_verilog_wire_wire_set_h_

#include "writer/verilog/common.h"

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
  ACCESSOR_NOTIFY_ACCESSOR,
};

class SignalInfo {
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
  SignalInfo *sig_info_;
  IResource *accessor_res_;
  AccessorInfo *accessor_info_;
};

class WireSet;

class AccessorInfo {
public:
  AccessorInfo(WireSet *wire_set,
	       IResource *accessor,
	       const string &name);

  void AddSignal(const string &name, AccessorSignalType type, int width);
  const vector<AccessorSignal> &GetSignals();
  const string &GetName();
  AccessorSignal *FindSignal(const SignalInfo &sig);

private:
  WireSet *wire_set_;
  IResource *accessor_;
  string name_;
  vector<AccessorSignal> accessor_signals_;
};

class WireSet {
public:
  WireSet(Resource &res, const string &name);
  ~WireSet();

  AccessorInfo *AddAccessor(IResource *accessor, const string &name);
  SignalInfo *GetSignalInfo(const string &name, AccessorSignalType type, int width);

  void Build();

private:
  void BuildAccessorWire(const SignalInfo &sig_info);
  void BuildResourceWire();
  void BuildArbitration(const SignalInfo &rsig_info, const SignalInfo &asig_info);
  void BuildRegisteredReq(const SignalInfo &rsig_info,
			  vector<AccessorInfo *> &handshake_accessors);
  void BuildAccessorAck(const SignalInfo &rsig_info, const SignalInfo &asig_info,
			vector<AccessorInfo *> &handshake_accessors);
  string ResourceWireName(const SignalInfo &sig_info);
  string AccessorWireName(const AccessorSignal &sig);
  string AccessorWireNameWithReg(const AccessorSignal &sig);

  Resource &res_;
  string name_;
  vector<AccessorInfo *> accessors_;
  map<string, SignalInfo *> signal_info_;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_wire_set_h_
