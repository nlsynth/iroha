#include "numeric/numeric_type.h"

namespace iroha {

NumericWidth::NumericWidth() : width_(32), is_signed_(false) {
}

int NumericWidth::GetWidth() const {
  return width_;
}

void NumericWidth::SetWidth(int width) {
  width_ = width;
}

bool NumericWidth::IsSigned() const {
  return is_signed_;
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
