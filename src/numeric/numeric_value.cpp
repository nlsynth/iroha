#include "numeric/numeric_value.h"

namespace iroha {

void ExtraWideValue::Clear() {
  for (int i = 0; i < 32; ++i) {
    value_[i] = 0;
  }
}

}  // namespace iroha

