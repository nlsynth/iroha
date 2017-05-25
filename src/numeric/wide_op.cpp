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
  uint64_t tv[8];
  if (a64 > 0) {
    const uint64_t *sv = s.GetArray();
    if (left) {
      int j = 0;
      for (int i = a64; i < 8; ++i, ++j) {
	tv[i] = sv[j];
      }
      for (int k = 0; k < a64; ++k) {
	tv[k] = 0;
      }
    } else {
      int j = 0;
      for (int i = a64; i < 8; ++i, ++j) {
	tv[j] = sv[i];
      }
      for (int k = j; k < 8; ++k) {
	tv[k] = 0;
      }
    }
  } else {
    const uint64_t *sv = s.GetArray();
    for (int i = 0; i < 8; ++i) {
      tv[i] = sv[i];
    }
  }

  int a1 = a % 64;
  uint64_t *rv = res->GetMutableArray();
  if (a1 > 0) {
    if (left) {
      uint64_t carry = 0;
      for (int i = 0; i < 8; ++i) {
	rv[i] = carry | (tv[i] << a1);
	carry = tv[i] >> (64 - a1);
      }
    } else {
      uint64_t mask = (~0) >> (64 - a1);
      uint64_t carry = 0;
      for (int i = 7; i >= 0; --i) {
	rv[i] = carry | (tv[i] >> a1);
	carry = (tv[i] & mask) << (64 - a1);
      }
    }
  } else {
    for (int i = 0; i < 8; ++i) {
      rv[i] = tv[i];
    }
  }
}

}  // namespace iroha
