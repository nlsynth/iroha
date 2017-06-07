#include "numeric/numeric_type.h"

#include <iomanip>
#include <sstream>

namespace iroha {

NumericWidth::NumericWidth() : is_signed_(false), width_(32) {
  SetWidth(width_);
}

NumericWidth::NumericWidth(bool is_signed, int width)
  : is_signed_(is_signed), width_(width) {
  SetWidth(width_);
}

void NumericWidth::SetWidth(int width) {
  width_ = width;
  mask_ = ~0;
  if (width_ < 64) {
    mask_ >>= (64 - width_);
  }
  if (width_ > 0) {
    value_count_ = ((width_ - 1) / 64) + 1;
  } else {
    value_count_ = 1;
  }
  if (value_count_ > 1) {
    is_wide_ = true;
  } else {
    is_wide_ = false;
  }
}

void NumericWidth::SetIsSigned(bool is_signed) {
  is_signed_ = is_signed;
}

NumericWidth NumericWidth::CommonWidth(const NumericWidth &w1,
				       const NumericWidth &w2) {
  bool is_signed;
  int int_part = w1.GetWidth();
  is_signed = w1.IsSigned() || w2.IsSigned();
  if (w2.GetWidth() > w1.GetWidth()) {
    int_part = w2.GetWidth();
  }
  return iroha::NumericWidth(is_signed, int_part);
}

std::string NumericWidth::Format() const {
  std::stringstream ss;
  ss << "<";
  if (IsSigned()) {
    ss << "+,";
  }
  ss << GetWidth() << ">";
  return ss.str();
}

Numeric::Numeric() {
  value_[0] = 0;
}

void Numeric::SetValue(uint64_t value) {
  value_[0] = value;
}

std::string Numeric::Format() const {
  if (type_.IsWide()) {
    int w = type_.GetWidth() / 64;
    std::stringstream ss;
    bool first = true;
    for (int i = w - 1; i >= 0; --i) {
      if (!first) {
	ss << "_";
      }
      ss << std::hex << std::setw(16) << std::setfill('0');
      ss << value_[i];
      first = false;
    }
    return ss.str();
  } else {
    uint64_t v = value_[0];
    std::string s;
    if (type_.IsSigned() && (v & (1 << (type_.GetWidth() - 1)))) {
      s = "-";
      v = (~v) + 1;
    }
    std::stringstream ss;
    ss << v;
    return s + ss.str();
  }
}

}  // namespace iroha
