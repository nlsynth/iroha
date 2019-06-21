#include "numeric/numeric_op.h"

#include "numeric/wide_op.h"

namespace iroha {

NumericWidth Op::ValueWidth(const Numeric &src_num) {
  bool is_signed = false;
  Numeric num = src_num;
  Numeric zero;
  Op::MakeConst0(0, zero.GetMutableArray());
  if (Compare(COMPARE_LT, num, zero)) {
    Numeric tmp = num;
    // negate
    Minus(tmp, &num);
    is_signed = true;
  }
  uint64_t n;
  n = num.GetValue0();
  int w;
  for (w = 0; n > 0; w++, n /= 2);
  return NumericWidth(is_signed, w);
}

bool Op::IsZero(const Numeric &n) {
  if (n.type_.IsWide()) {
    return WideOp::IsZero(n);
  }
  return n.GetValue0() == 0;
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
		   Numeric *res) {
  if (x.type_.IsWide()) {
    switch (op) {
    case BINOP_LSHIFT:
    case BINOP_RSHIFT:
      {
	int c = y.GetValue0();
	WideOp::Shift(x.GetArray(), c, (op == BINOP_LSHIFT),
		      res->GetMutableArray());
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

void Op::Minus(const Numeric &x, Numeric *res) {
  *res = x;
  res->SetValue0(res->GetValue0() * -1);
}

void Op::Clear(Numeric *res) {
  if (res->type_.IsWide()) {
    NumericValue *v = res->GetMutableArray();
    for (int i = 0; i < 8; ++i) {
      v->value_[i] = 0;
    }
  } else {
    res->SetValue0(0);
  }
}

bool Op::Compare(CompareOp op, const Numeric &x, const Numeric &y) {
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

void Op::BitInv(const Numeric &num, Numeric *res) {
  *res = num;
  res->SetValue0(~res->GetValue0());
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

void Op::SelectBits(const Numeric &num, int h, int l,
		    Numeric *res) {
  int width = h - l + 1;
  if (num.type_.IsWide()) {
    WideOp::SelectBits(num, h, l, res);
    res->type_ = NumericWidth(false, width);
    return;
  }
  *res = num;
  res->SetValue0(0);
  for (int i = 0; i < width; ++i) {
    if ((1UL << (l + i)) & num.GetValue0()) {
      res->SetValue0(res->GetValue0() | (1UL << i));
    }
  }
  res->type_ = NumericWidth(false, width);
}

void Op::Concat(const Numeric &x, const Numeric &y,
		Numeric *a) {
  NumericWidth w = NumericWidth(false,
				x.type_.GetWidth() + y.type_.GetWidth());
  if (w.IsWide()) {
    WideOp::Concat(x, y, a);
    return;
  }
  a->SetValue0((x.GetValue0() << y.type_.GetWidth()) + y.GetValue0());
  a->type_ = w;
}

}  // namespace iroha
