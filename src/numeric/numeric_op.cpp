#include "numeric/numeric_op.h"

#include "numeric/wide_op.h"

namespace iroha {

NumericWidth Op::ValueWidth(const Numeric &src_num) {
  bool is_signed = false;
  Numeric num = src_num;
  Numeric zero;
  Op::MakeConst(0, &zero);
  if (Compare(COMPARE_LT, num, zero)) {
    Numeric tmp = num;
    // negate
    Minus(tmp, &num);
    is_signed = true;
  }
  uint64_t n;
  n = num.GetValue();
  int w;
  for (w = 0; n > 0; w++, n /= 2);
  return NumericWidth(is_signed, w);
}

bool Op::IsZero(const Numeric &n) {
  if (n.type_.IsWide()) {
    return WideOp::IsZero(n);
  }
  return n.GetValue() == 0;
}

void Op::Add(const Numeric &x, const Numeric &y, Numeric *a) {
  a->SetValue(x.GetValue() + y.GetValue());
}

void Op::Sub(const Numeric &x, const Numeric &y, Numeric *a) {
  a->SetValue(x.GetValue() - y.GetValue());
}

void Op::MakeConst(uint64_t value, Numeric *num) {
  num->SetValue(value);
}

void Op::CalcBinOp(BinOp op, const Numeric &x, const Numeric &y,
		   Numeric *res) {
  switch (op) {
  case BINOP_LSHIFT:
  case BINOP_RSHIFT:
    {
      if (x.type_.IsWide()) {
	int c = y.GetValue();
	WideOp::Shift(x, c, (op == BINOP_LSHIFT), res);
      } else {
	int c = y.GetValue();
	if (op == BINOP_LSHIFT) {
	  res->SetValue(x.GetValue() << c);
	} else {
	  res->SetValue(x.GetValue() >> c);
	}
      }
    }
    break;
  case BINOP_AND:
    res->SetValue(x.GetValue() & y.GetValue());
    break;
  case BINOP_OR:
    res->SetValue(x.GetValue() | y.GetValue());
    break;
  case BINOP_XOR:
    res->SetValue(x.GetValue() ^ y.GetValue());
    break;
  case BINOP_MUL:
    res->SetValue(x.GetValue() * y.GetValue());
    break;
  }
}

void Op::Minus(const Numeric &x, Numeric *res) {
  *res = x;
  res->SetValue(res->GetValue() * -1);
}

void Op::Clear(Numeric *res) {
  if (res->type_.IsWide()) {
    uint64_t *v = res->GetMutableArray();
    for (int i = 0; i < 8; ++i) {
      v[i] = 0;
    }
  } else {
    res->SetValue(0);
  }
}

bool Op::Compare(CompareOp op, const Numeric &x, const Numeric &y) {
  switch (op) {
  case COMPARE_LT:
    return x.GetValue() < y.GetValue();
  case COMPARE_GT:
    return x.GetValue() > y.GetValue();
  case COMPARE_EQ:
    return x.GetValue() == y.GetValue();
  default:
    break;
  }
  return true;
}

void Op::BitInv(const Numeric &num, Numeric *res) {
  *res = num;
  res->SetValue(~res->GetValue());
}

void Op::FixupWidth(const NumericWidth &w, Numeric *num) {
  num->SetValue(num->GetValue() & w.GetMask());
  num->type_ = w;
}

void Op::SelectBits(const Numeric &num, int h, int l,
		    Numeric *res) {
  *res = num;
  res->SetValue(0);
  int width = h - l + 1;
  for (int i = 0; i < width; ++i) {
    if ((1UL << (l + i)) & num.GetValue()) {
      res->SetValue(res->GetValue() | (1UL << i));
    }
  }
  res->type_ = NumericWidth(false, width);
}

void Op::Concat(const Numeric &x, const Numeric &y,
		Numeric *a) {
  a->SetValue((x.GetValue() << y.type_.GetWidth()) + y.GetValue());
  a->type_ = NumericWidth(false,
			  x.type_.GetWidth() + y.type_.GetWidth());
}

}  // namespace iroha
