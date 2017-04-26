#include "numeric/numeric_type.h"

namespace iroha {

Numeric::Numeric() : value_(0) {
}

uint64_t Numeric::GetValue() const {
  return value_;
}

void Numeric::SetValue(uint64_t value) {
  value_ = value;
}

}  // namespace iroha
