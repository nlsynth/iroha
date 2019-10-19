#include "writer/verilog/wire/accessor_info.h"

#include "iroha/stl_util.h"
#include "writer/verilog/wire/wire_set.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

bool SignalDescription::IsUpstream() const {
  switch (type_) {
  case ACCESSOR_REQ: return true;
  case ACCESSOR_ACK: return false;
  case ACCESSOR_READ_ARG: return false;
  case ACCESSOR_WRITE_ARG:return true;
  case ACCESSOR_NOTIFY_PARENT: return true;
  case ACCESSOR_NOTIFY_PARENT_SECONDARY: return true;
  case ACCESSOR_NOTIFY_ACCESSOR: return false;
  }
  return false;
}

AccessorInfo::AccessorInfo(WireSet *wire_set, const IResource *accessor)
  : wire_set_(wire_set), accessor_(accessor), distance_(0) {
}

AccessorInfo::~AccessorInfo() {
  STLDeleteValues(&accessor_signals_);
}

AccessorSignal *AccessorInfo::AddSignal(const string &name,
					AccessorSignalType type,
					int width) {
  AccessorSignal *asig = new AccessorSignal();
  asig->sig_desc_ = wire_set_->GetSignalDescription(name, type, width);
  asig->accessor_res_ = accessor_;
  asig->accessor_info_ = this;
  accessor_signals_.push_back(asig);
  return asig;
}

const vector<AccessorSignal *> &AccessorInfo::GetSignals() const {
  return accessor_signals_;
}

AccessorSignal *AccessorInfo::FindSignal(const SignalDescription &desc) const {
  for (auto *asig : accessor_signals_) {
    if (asig->sig_desc_ == &desc) {
      return asig;
    }
  }
  return nullptr;
}

void AccessorInfo::SetDistance(int distance) {
  distance_ = distance;
}

int AccessorInfo::GetDistance() const {
  return distance_;
}

const IResource *AccessorInfo::GetResource() const {
  return accessor_;
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
