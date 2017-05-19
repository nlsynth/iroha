// -*- C++ -*-
#ifndef _numeric_numeric_type_h_
#define _numeric_numeric_type_h_

#include <cstdint>

namespace iroha {

class NumericWidth {
public:
  NumericWidth();
  NumericWidth(bool is_signed, int width);

  // Width 0 means a scalar value e.g. 'reg v;'.
  int GetWidth() const {
    return width_;
  }
  void SetWidth(int width);
  bool IsSigned() const {
    return is_signed_;
  }
  void SetIsSigned(bool is_signed);
  uint64_t GetMask() const {
    return mask_;
  }

private:
  bool is_signed_;
  int width_;
  uint64_t mask_;
};

class Numeric {
public:
  Numeric();

  uint64_t GetValue() const {
    return value_;
  }
  void SetValue(uint64_t value);

  NumericWidth type_;

private:
  uint64_t value_;
};

}  // namespace iroha

#endif  // _numeric_numeric_type_h_
