#include "numeric/numeric_type.h"

namespace iroha {

NumericWidth::NumericWidth() : is_signed_(false), width_(32) {
  SetWidth(width_);
}

NumericWidth::NumericWidth(bool is_signed, int width)
  : is_signed_(is_signed), width_(width) {
}

void NumericWidth::SetWidth(int width) {
  width_ = width;
  mask_ = ~0;
  if (width_ < 64) {
    mask_ >>= (64 - width_);
  }
}

void NumericWidth::SetIsSigned(bool is_signed) {
  is_signed_ = is_signed;
}

Numeric::Numeric() : value_(0) {
}

void Numeric::SetValue(uint64_t value) {
  value_ = value;
}

}  // namespace iroha
