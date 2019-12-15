#include "numeric/numeric_op.h"

#include "numeric/wide_op.h"

namespace iroha {

bool Op::IsZero(const NumericWidth &w, const NumericValue &n) {
  if (w.IsWide()) {
    return WideOp::IsZero(w, n);
  }
  return (n.GetValue0() & w.GetMask()) == 0;
}

void Op::Add0(const NumericValue &x, const NumericValue &y, NumericValue *a) {
  a->SetValue0(x.GetValue0() + y.GetValue0());
}

void Op::Sub0(const NumericValue &x, const NumericValue &y, NumericValue *a) {
  a->SetValue0(x.GetValue0() - y.GetValue0());
}

void Op::MakeConst0(uint64_t value, NumericValue *v) {
  v->SetValue0(value);
}

void Op::CalcBinOp(BinOp op, const NumericValue &x, const NumericValue &y,
		   const NumericWidth &w, NumericValue *res) {
  if (w.IsWide()) {
    switch (op) {
    case BINOP_LSHIFT:
    case BINOP_RSHIFT:
      {
	int c = y.GetValue0();
	if (w.IsExtraWide()) {
	  WideOp::ShiftExtraWide(x, c, (op == BINOP_LSHIFT), res);
	} else {
	  WideOp::Shift(x, c, (op == BINOP_LSHIFT), res);
	}
      }
      break;
    case BINOP_AND:
    case BINOP_OR:
    case BINOP_XOR:
      {
	WideOp::BinBitOp(op, x, y, res);
      }
      break;
    case BINOP_MUL:
    case BINOP_DIV:
      // These are not usable for wide nums.
      return;
      break;
    }
    return;
  }
  switch (op) {
  case BINOP_LSHIFT:
  case BINOP_RSHIFT:
    {
      int c = y.GetValue0();
      if (op == BINOP_LSHIFT) {
	res->SetValue0(x.GetValue0() << c);
      } else {
	res->SetValue0(x.GetValue0() >> c);
      }
    }
    break;
  case BINOP_AND:
    res->SetValue0(x.GetValue0() & y.GetValue0());
    break;
  case BINOP_OR:
    res->SetValue0(x.GetValue0() | y.GetValue0());
    break;
  case BINOP_XOR:
    res->SetValue0(x.GetValue0() ^ y.GetValue0());
    break;
  case BINOP_MUL:
    res->SetValue0(x.GetValue0() * y.GetValue0());
    break;
  case BINOP_DIV:
    res->SetValue0(x.GetValue0() / y.GetValue0());
    break;
  }
}

void Op::Minus0(const NumericValue &x, NumericValue *res) {
  *res = x;
  res->SetValue0(res->GetValue0() * -1);
}

void Op::Clear(NumericWidth &w, NumericValue *val) {
  if (w.IsWide()) {
    if (w.IsExtraWide()) {
      val->extra_wide_value_->Clear();
    } else {
      for (int i = 0; i < 8; ++i) {
	val->value_[i] = 0;
      }
    }
  } else {
    val->SetValue0(0);
  }
}

void Op::Set(const NumericWidth &sw, const NumericValue &src,
	     const NumericWidth &dw, NumericValue *dst) {
  const uint64_t *sv;
  if (sw.IsExtraWide()) {
    sv = src.extra_wide_value_->value_;
  } else {
    sv = src.value_;
  }
  uint64_t *dv;
  if (dw.IsExtraWide()) {
    dv = dst->extra_wide_value_->value_;
  } else {
    dv = dst->value_;
  }
  int bits = sw.GetWidth();
  if (dw.GetWidth() < bits) {
    bits = dw.GetWidth();
  }
  int a = bits / 64;
  for (int i = 0; i < a; ++i) {
    dv[i] = sv[i];
  }
  int r = bits % 64;
  if (r > 0) {
    uint64_t mask = ~((~0ULL) << r);
    dv[a] = sv[a] | mask;
  }
}

bool Op::Eq(const NumericWidth &w, const NumericValue &v1,
	    const NumericValue &v2) {
  const uint64_t *s1;
  const uint64_t *s2;
  if (w.IsExtraWide()) {
    s1 = v1.extra_wide_value_->value_;
    s2 = v2.extra_wide_value_->value_;
  } else {
    s1 = v1.value_;
    s2 = v2.value_;
  }
  int bits = w.GetWidth();
  int a = bits / 64;
  for (int i = 0; i < a; ++i) {
    if (s1[i] != s2[i]) {
      return false;
    }
  }
  int r = bits % 64;
  if (r > 0) {
    uint64_t mask = ~((~0ULL) << r);
    if ((s1[a] & mask) != (s2[a] & mask)) {
      return false;
    }
  }
  return true;
}

bool Op::Compare0(CompareOp op, const NumericValue &x, const NumericValue &y) {
  switch (op) {
  case COMPARE_LT:
    return x.GetValue0() < y.GetValue0();
  case COMPARE_GT:
    return x.GetValue0() > y.GetValue0();
  case COMPARE_EQ:
    return x.GetValue0() == y.GetValue0();
  default:
    break;
  }
  return true;
}

void Op::BitInv0(const NumericValue &num, NumericValue *res) {
  res->SetValue0(~(num.GetValue0()));
}

void Op::FixupValueWidth(const NumericWidth &w, NumericValue *val) {
  if (w.IsWide()) {
    WideOp::FixupWidth(w, val);
    return;
  }
  // Non wide.
  val->value_[0] = val->value_[0] & w.GetMask();
}

void Op::SelectBits(const NumericValue &num, const NumericWidth &num_width,
		    int h, int l,
		    NumericValue *res, NumericWidth *res_width) {
  int width = h - l + 1;
  if (res_width != nullptr) {
    *res_width = NumericWidth(false, width);
  }
  if (num_width.IsWide()) {
    WideOp::SelectBits(num, num_width, h, l, res);
    return;
  }
  uint64_t o = 0;
  uint64_t v = num.GetValue0();
  for (int i = 0; i < width; ++i) {
    if ((1UL << (l + i)) & v) {
      o = o | (1UL << i);
    }
  }
  res->SetValue0(o);
}

void Op::Concat(const NumericValue &x, const NumericWidth &xw,
		const NumericValue &y, const NumericWidth &yw,
		NumericValue *a, NumericWidth *aw) {
  NumericWidth w(false, xw.GetWidth() + yw.GetWidth());
  if (w.IsWide()) {
    WideOp::Concat(x, xw, y, yw, a, aw);
    return;
  }
  a->SetValue0((x.GetValue0() << yw.GetWidth()) + y.GetValue0());
  if (aw != nullptr) {
    *aw = w;
  }
}

}  // namespace iroha
