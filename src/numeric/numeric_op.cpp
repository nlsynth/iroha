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

void Op::CalcBinOp(BinOp op, const Numeric &x, const Numeric &y,
		   NumericValue *res) {
  if (x.type_.IsWide()) {
    switch (op) {
    case BINOP_LSHIFT:
    case BINOP_RSHIFT:
      {
	int c = y.GetValue0();
	WideOp::Shift(x.GetArray(), c, (op == BINOP_LSHIFT), res);
      }
      break;
    case BINOP_AND:
    case BINOP_OR:
    case BINOP_XOR:
      {
	WideOp::BinBitOp(op, x.GetArray(), y.GetArray(), res);
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
    for (int i = 0; i < 8; ++i) {
      val->value_[i] = 0;
    }
  } else {
    val->SetValue0(0);
  }
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

void Op::FixupWidth(const NumericWidth &w, Numeric *num) {
  FixupValueWidth(w, num->GetMutableArray());
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
    WideOp::SelectBits(num, h, l, res);
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
