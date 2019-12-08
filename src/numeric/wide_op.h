// -*- C++ -*-
#ifndef _numeric_wide_op_h_
#define _numeric_wide_op_h_

#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"

namespace iroha {

class WideOp {
public:
  static bool IsZero(const NumericWidth &w, const NumericValue &val);
  static void Shift(const NumericValue &s, int amount, bool left,
		    NumericValue *res);
  static void ShiftExtraWide(const NumericValue &s, int amount, bool left,
			     NumericValue *res);
  static void BinBitOp(enum BinOp op, const NumericValue &x,
		       const NumericValue &y, NumericValue *res);
  static void SelectBits(const NumericValue &val, const NumericWidth &w,
			 int h, int l,
			 NumericValue *res);
  static void Concat(const NumericValue &x, const NumericWidth &xw,
		     const NumericValue &y, const NumericWidth &yw,
		     NumericValue *a, NumericWidth *aw);
  static void FixupWidth(const NumericWidth &w, NumericValue *val);

private:
  static void ShiftArray(const uint64_t *sv, int len, int amount, bool left,
			 uint64_t *tv);
  static void ShiftLocal(const uint64_t *tv, int len, int amount, bool left,
			 uint64_t *rv);
};

}  // namespace iroha

#endif  // _numeric_wide_op_h_
