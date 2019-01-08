// -*- C++ -*-
#ifndef _writer_verilog_wire_set_h_
#define _writer_verilog_wire_set_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

enum AccessorSignalType : int {
  ACCESSOR_REQ,
  ACCESSOR_ACK,
  ACCESSOR_READ_ARG,
  ACCESSOR_WRITE_ARG,
  ACCESSOR_NOTIFY_PARENT,
  ACCESSOR_NOTIFY_ACCESSOR,
};

class AccessorInfo;

class AccessorSignal {
public:
  // e.g. ack, req...
  string name_;
  AccessorSignalType type_;
  int width_;
  IResource *accessor_res_;
  AccessorInfo *info_;
};

class AccessorInfo {
public:
  AccessorInfo(IResource *accessor,
	       const string &name);

  void AddSignal(const string &name, AccessorSignalType type, int width);
  const vector<AccessorSignal> &GetSignals();
  const string &GetName();
  AccessorSignal *FindSignal(const string &name);

private:
  IResource *accessor_;
  string name_;
  vector<AccessorSignal> signals_;
};

class WireSet {
public:
  WireSet(Resource &res, const string &name);
  ~WireSet();

  AccessorInfo *AddAccessor(IResource *accessor, const string &name);

  void Build();

private:
  void BuildAccessorWire(const AccessorSignal &primary_sig);
  void BuildResourceWire(const vector<AccessorSignal> &uniq_signals);
  void BuildArbitration(const AccessorSignal &req, const AccessorSignal &ack);
  string ResourceWireName(const AccessorSignal &sig);
  string AccessorWireName(const AccessorSignal &sig);

  Resource &res_;
  string name_;
  vector<AccessorInfo *> accessors_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_set_h_
