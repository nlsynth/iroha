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
  string GetMuxName() const;

  void Build();

  string AccessorWireName(const AccessorSignal &sig);
  string AccessorEdgeWireName(const AccessorSignal &sig) const;
  string ResourceWireName(const SignalDescription &desc) const;

  const vector<AccessorInfo *> &GetAccessors() const;
  vector<SignalDescription *> GetSignals() const;

private:
  void BuildAccessorWire(const SignalDescription &desc);
  void BuildResourceWire();
  void BuildDistanceRegs();

  Resource &res_;
  string resource_name_;
  vector<AccessorInfo *> accessors_;
  // Keyed by "req", "ack" and so on.
  map<string, SignalDescription *> signal_desc_;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_wire_set_h_
