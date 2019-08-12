#include "numeric/wide_op.h"

namespace iroha {

bool WideOp::IsZero(const NumericWidth &w, const NumericValue &val) {
  const uint64_t *v;
  int s;
  if (w.IsExtraWide()) {
    s = 32;
    v = val.extra_wide_value_->value_;
  } else {
    s = 8;
    v = val.value_;
  }
  for (int i = 0; i < s; ++i) {
    if (v[i] > 0) {
      return false;
    }
  }
  return true;
}

void WideOp::Shift(const NumericValue &s, int amount, bool left,
		   NumericValue *res) {
  int a64 = amount / 64;
  uint64_t tv[8];
  if (a64 > 0) {
    const uint64_t *sv = s.value_;
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
    const uint64_t *sv = s.value_;
    for (int i = 0; i < 8; ++i) {
      tv[i] = sv[i];
    }
  }

  int a1 = amount % 64;
  uint64_t *rv = res->value_;
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

void WideOp::BinBitOp(enum BinOp op, const NumericValue &x,
		      const NumericValue &y, NumericValue *res) {
  uint64_t *rv = res->value_;
  const uint64_t *xv = x.value_;
  const uint64_t *yv = y.value_;
  switch (op) {
  case BINOP_AND:
    {
      for (int i = 0; i < 8; ++i) {
	rv[i] = xv[i] & yv[i];
      }
    }
    break;
  case BINOP_OR:
    {
      for (int i = 0; i < 8; ++i) {
	rv[i] = xv[i] | yv[i];
      }
    }
    break;
  case BINOP_XOR:
    {
      for (int i = 0; i < 8; ++i) {
	rv[i] = xv[i] ^ yv[i];
      }
    }
    break;
  default:
    break;
  }
}

void WideOp::SelectBits(const NumericValue &val, int h, int l,
			NumericValue *res) {
  Shift(val, l, false, res);
}

void WideOp::Concat(const NumericValue &x, const NumericWidth &xw,
		    const NumericValue &y, const NumericWidth &yw,
		    NumericValue *a, NumericWidth *aw) {
  NumericValue tmp;
  Shift(x, yw.GetWidth(), true, &tmp);
  NumericValue yy = y;
  FixupWidth(yw, &yy);
  BinBitOp(BINOP_OR, tmp, yy, a);
  if (aw != nullptr) {
    *aw = NumericWidth(false, xw.GetWidth() + yw.GetWidth());
  }
}

void WideOp::FixupWidth(const NumericWidth &w, NumericValue *val) {
  uint64_t mask = ~0;
  mask >>= (64 - (w.GetWidth() % 64));
  int value_count = w.GetValueCount();
  uint64_t *rv;
  int s;
  if (w.IsExtraWide()) {
    rv = val->extra_wide_value_->value_;
    s = 32;
  } else {
    rv = val->value_;
    s = 8;
  }
  for (int i = value_count; i < s; ++i) {
    rv[i] = 0;
  }
  rv[value_count - 1] &= mask;
}

}  // namespace iroha
