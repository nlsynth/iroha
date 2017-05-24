#include "numeric/wide_op.h"

namespace iroha {

bool WideOp::IsZero(const Numeric &n) {
  const uint64_t *v = n.GetArray();
  for (int i = 0; i < 8; ++i) {
    if (v[i] > 0) {
      return false;
    }
  }
  return true;
}

void WideOp::Shift(const Numeric &s, int a, bool left, Numeric *res) {
  int a64 = a / 64;
  int d = 0;
  if (a64 > 0) {
    const uint64_t *sv = s.GetArray();
    uint64_t *rv = res->GetMutableArray();
    if (left) {
      int j = 0;
      for (int i = a64; i < 8; ++i, ++j) {
	rv[i] = sv[j];
      }
      for (int k = 0; k < j; ++k) {
	rv[k] = 0;
      }
    } else {
      int j = 0;
      for (int i = a64; i < 8; ++i, ++j) {
	rv[j] = sv[i];
      }
      for (int k = j - 1; k < 8; ++k) {
	rv[k] = 0;
      }
    }
  }
  // TODO shift remainder.
}

}  // namespace iroha
