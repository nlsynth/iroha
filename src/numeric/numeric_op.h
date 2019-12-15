// -*- C++ -*-
#ifndef _numeric_numeric_op_h_
#define _numeric_numeric_op_h_

#include "numeric/numeric_type.h"

namespace iroha {

enum CompareOp {
  COMPARE_LT,
  COMPARE_GT,
  COMPARE_EQ,
};

enum BinOp {
  BINOP_LSHIFT,
  BINOP_RSHIFT,
  BINOP_AND,
  BINOP_OR,
  BINOP_XOR,
  BINOP_MUL,
  BINOP_DIV,
};

// NOTE:
// * Arithmetic operations wider than 64bits may not be implemented.
// * Must not rely on output arg's .type_.
// * Do not rely on input arg's .type_ as much as possible.
// TODO: Add width argument to methods
// (or even take NumericValue instead of Numeric.)
class Op {
public:
  static bool IsZero(const NumericWidth &w, const NumericValue &n);
  // *0 methods can be used up to 64bit.
  static void Add0(const NumericValue &x, const NumericValue &y,
		   NumericValue *a);
  static void Sub0(const NumericValue &x, const NumericValue &y,
		   NumericValue *a);
  static void MakeConst0(uint64_t value, NumericValue *v);
  static void Minus0(const NumericValue &val, NumericValue *res);
  static bool Compare0(enum CompareOp op, const NumericValue &x,
		       const NumericValue &y);
  static bool Eq(const NumericWidth &w, const NumericValue &v1,
		 const NumericValue &v2);
  static void BitInv0(const NumericValue &num, NumericValue *res);

  static void Clear(NumericWidth &w, NumericValue *val);
  static void Set(const NumericWidth &sw, const NumericValue &src,
		  const NumericWidth &dw, NumericValue *dst);
  static void CalcBinOp(enum BinOp op,
			const NumericValue &x, const NumericValue &y,
			const NumericWidth &w,
			NumericValue *res);
  static void Concat(const NumericValue &x, const NumericWidth &xw,
		     const NumericValue &y, const NumericWidth &yw,
		     NumericValue *a, NumericWidth *aw);
  static void SelectBits(const NumericValue &num, const NumericWidth &num_width,
			 int h, int l,
			 NumericValue *res, NumericWidth *res_width);
  // cuts upper bits
  static void FixupValueWidth(const NumericWidth &w, NumericValue *val);
};

}  // namespace iroha

#endif  // _numeric_numeric_op_h_
