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
};

class Op {
public:
  static bool IsZero(const Numeric &n);
  // Gets required width to store given number.
  static NumericWidth ValueWidth(const Numeric &n);
  static void Add(const Numeric &x, const Numeric &y, Numeric *a);
  static void Sub(const Numeric &x, const Numeric &y, Numeric *a);
  static void Minus(const Numeric &num, Numeric *res);
  static bool Compare(enum CompareOp op, const Numeric &x, const Numeric &y);
  static void CalcBinOp(enum BinOp op, const Numeric &x, const Numeric &y,
			Numeric *res);
  static void MakeConst(uint64_t value, Numeric *num);
  static void Concat(const Numeric &x, const Numeric &y, Numeric *a);
  static void SelectBits(const Numeric &num, int h, int l, Numeric *res);
  static void BitInv(const Numeric &num, Numeric *res);
  // cuts upper bits
  static void FixupWidth(const NumericWidth &w, Numeric *num);
};

}  // namespace iroha

#endif  // _numeric_numeric_type_h_
