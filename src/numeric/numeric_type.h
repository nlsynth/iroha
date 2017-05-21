// -*- C++ -*-
#ifndef _numeric_numeric_type_h_
#define _numeric_numeric_type_h_

#include <cstdint>

namespace iroha {

class NumericWidth {
public:
  NumericWidth();
  NumericWidth(bool is_signed, int width);

  static NumericWidth CommonWidth(const NumericWidth &w1,
				  const NumericWidth &w2);

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
  int GetValueCount() {
    return value_count_;
  }

private:
  bool is_signed_;
  int value_count_;
  int width_;
  uint64_t mask_;
};

class Numeric {
public:
  Numeric();

  uint64_t GetValue() const {
    return value_[0];
  }
  void SetValue(uint64_t value);

  NumericWidth type_;

private:
  uint64_t value_[8];
};

}  // namespace iroha

#endif  // _numeric_numeric_type_h_
