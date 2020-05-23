// -*- C++ -*-
#ifndef _numeric_numeric_literal_h_
#define _numeric_numeric_literal_h_

#include <string>
#include <cstdint>

namespace iroha {

class Numeric;
class NumericManager;

// This struct can be a part of YYSTYPE union.
struct NumericLiteral {
  uint64_t value;
  // non negative if defined.
  int width;
  bool has_error;

  void ToNumeric(NumericManager *mgr, Numeric *numeric);

  static NumericLiteral Parse(const std::string &token);

private:
  static uint64_t Parse0b(const std::string &token);
  static std::string RemoveUnderscore(const std::string &s);
};

}  // namespace iroha

#endif  // _numeric_numeric_h_
