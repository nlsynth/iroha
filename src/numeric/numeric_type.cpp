#include "numeric/numeric_type.h"

namespace iroha {

Numeric::Numeric() : value_(0) {
}

void Numeric::SetValue(uint64_t value) {
  value_ = value;
}

}  // namespace iroha
