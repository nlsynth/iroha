// -*- C++ -*-
#ifndef _numeric_wide_op_h_
#define _numeric_wide_op_h_

#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"

namespace iroha {

class WideOp {
public:
  static bool IsZero(const Numeric &n);
  static void Shift(const NumericValue &s, int amount, bool left,
		    NumericValue *res);
  static void BinBitOp(enum BinOp op, const Numeric &x, const Numeric &y,
		       Numeric *res);
  static void SelectBits(const Numeric &num, int h, int l, Numeric *res);
  static void Concat(const Numeric &x, const Numeric &y, Numeric *a);
  static void FixupWidth(const NumericWidth &w, NumericValue *val);
};

}  // namespace iroha

#endif  // _numeric_wide_op_h_
