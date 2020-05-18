// -*- C++ -*-
#ifndef _numeric_numeric_type_h_
#define _numeric_numeric_type_h_

#include <cstdint>
#include <string>
#include <iostream>

namespace iroha {

class NumericManager;

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
  int GetValueCount() const {
    return value_count_;
  }
  bool IsWide() const {
    return is_wide_;
  }
  bool IsExtraWide() const {
    return is_extra_wide_;
  }
  std::string Format() const;

private:
  bool is_signed_;
  bool is_wide_;
  bool is_extra_wide_;
  int value_count_;
  int width_;
  uint64_t mask_;
};

}  // namespace iroha

#endif  // _numeric_numeric_type_h_
