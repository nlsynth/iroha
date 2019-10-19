// -*- C++ -*-
#ifndef _writer_verilog_wire_accessor_info_
#define _writer_verilog_wire_accessor_info_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class SignalDescription {
public:
  // e.g. ack, req...
  string name_;
  AccessorSignalType type_;
  int width_;
  // Sets 0 to the arg if type_ == ACCESSOR_WRITE_ARG (e.g. wen).
  bool default0_;

  bool IsUpstream() const;
};

// Per accessor and signal.
class AccessorSignal {
public:
  SignalDescription *sig_desc_;
  const IResource *accessor_res_;
  AccessorInfo *accessor_info_;
};

class AccessorInfo {
public:
  AccessorInfo(WireSet *wire_set, const IResource *accessor);
  ~AccessorInfo();

  AccessorSignal *AddSignal(const string &name, AccessorSignalType type,
			    int width);
  const vector<AccessorSignal *> &GetSignals() const;
  AccessorSignal *FindSignal(const SignalDescription &desc) const;
  void SetDistance(int distance);
  int GetDistance() const;
  const IResource *GetResource() const;

private:
  WireSet *wire_set_;
  const IResource *accessor_;
  string accessor_name_;
  vector<AccessorSignal *> accessor_signals_;
  int distance_;
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_accessor_info_h_
