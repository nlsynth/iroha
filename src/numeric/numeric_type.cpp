#include "numeric/numeric_type.h"

#include "numeric/numeric_manager.h"
#include "numeric/numeric_op.h"

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
    int w = width_;
    if (w == 0) {
      // 0 means a scalar value.
      w = 1;
    }
    mask_ >>= (64 - w);
  }
  if (width_ > 0) {
    value_count_ = ((width_ - 1) / 64) + 1;
  } else {
    value_count_ = 1;
  }
  is_extra_wide_ = false;
  if (value_count_ > 1) {
    if (value_count_ > 8) {
      is_extra_wide_ = true;
    }
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

}  // namespace iroha
