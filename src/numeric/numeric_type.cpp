#include "numeric/numeric_type.h"

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

Numeric::Numeric() {
  value_[0] = 0;
}

void Numeric::SetValue(uint64_t value) {
  value_[0] = value;
}

}  // namespace iroha
