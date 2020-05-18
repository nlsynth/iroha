// -*- C++ -*-
#ifndef _numeric_numeric_value_h_
#define _numeric_numeric_value_h_

#include <cstdint>

namespace iroha {

class NumericManager;

class ExtraWideValue {
public:
  void Clear();

  uint64_t value_[32];
  NumericManager *owner_;
};

class NumericValue {
public:
  union {
    // 512 bit storage as default and use extra wide_values_ if the
    // width is wider than 512.
    // (We might change 512 to just 64 later)
    uint64_t value_[8];
    ExtraWideValue *extra_wide_value_;
  };
  // Only for non wide values.
  uint64_t GetValue0() const {
    return value_[0];
  }
  // Only for non wide values.
  void SetValue0(uint64_t v) {
    value_[0] = v;
  }
};

}  // namespace iroha

#endif  // _numeric_numeric_value_h_
