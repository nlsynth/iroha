#include "numeric/wide_op.h"

namespace iroha {

bool WideOp::IsZero(const Numeric &n) {
  const uint64_t *v;
  int s;
  if (n.type_.IsExtraWide()) {
    s = 32;
    v = n.GetArray().extra_wide_value_->value_;
  } else {
    s = 8;
    v = n.GetArray().value_;
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
  int d = 0;
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

void WideOp::BinBitOp(enum BinOp op, const Numeric &x, const Numeric &y, Numeric *res) {
  uint64_t *rv = res->GetMutableArray()->value_;
  const uint64_t *xv = x.GetArray().value_;
  const uint64_t *yv = y.GetArray().value_;
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
  }
}

void WideOp::SelectBits(const NumericValue &val, int h, int l,
			NumericValue *res) {
  Shift(val, l, false, res);
}

void WideOp::Concat(const Numeric &x, const Numeric &y, Numeric *a) {
  Numeric tmp;
  Shift(x.GetArray(), y.type_.GetWidth(), true, tmp.GetMutableArray());
  Numeric yy = y;
  FixupWidth(y.type_, yy.GetMutableArray());
  BinBitOp(BINOP_OR, tmp, yy, a);
  NumericWidth w = NumericWidth(false,
				x.type_.GetWidth() + y.type_.GetWidth());
  a->type_ = w;
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
