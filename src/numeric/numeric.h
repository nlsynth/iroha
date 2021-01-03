// -*- C++ -*-
#ifndef _numeric_numeric_h_
#define _numeric_numeric_h_

#include "numeric/numeric_value.h"
#include "numeric/numeric_width.h"

namespace iroha {

// Numeric = Value AND Width (namely NumericValue and NumericWidth).
class Numeric {
public:
  Numeric();
  Numeric(const NumericValue &v, const NumericWidth &w) : type_(w), value_(v) {
  }

  // Get/SetValue0 are convenient interface for small values.
  uint64_t GetValue0() const {
    if (type_.IsExtraWide()) {
      return value_.extra_wide_value_->value_[0];
    } else {
      return value_.value_[0];
    }
  }
  void SetValue0(uint64_t value);
  const NumericValue &GetValue() const {
    return value_;
  }
  NumericValue *GetMutableValue() {
    return &value_;
  }
  std::string Format() const;
  static std::string Format(const NumericWidth &w, const NumericValue &val);

  // Manages storage for big number.
  static NumericManager *DefaultManager();
  static NumericManager *CreateManager();
  static void ReleaseDefaultManager();
  static void DeleteManager(NumericManager *mgr);
  static void CopyValue(const Numeric &src, NumericManager *mgr, Numeric *dst);
  static void CopyValueWithWidth(const NumericValue &src_value,
				 const NumericWidth &src_width,
				 const NumericWidth &dst_width,
				 NumericManager *mgr,
				 NumericValue *value);
  static void MayPopulateStorage(const NumericWidth &width, NumericManager *mgr,
				 NumericValue *n);
  // Assumes n has value in the inline storage and n.type_ (maybe) is
  // extra wide. This also copies the data.
  static void MayExpandStorage(NumericManager *mgr, Numeric *n);
  static void Clear(const NumericWidth &width, NumericValue *value);

  NumericWidth type_;

private:
  NumericValue value_;

  static std::string FormatArray(const NumericWidth &type,
				 const uint64_t *v);
  static void FixupArray(int w, int l, uint64_t *v);
};

}  // namespace iroha

#endif  // _numeric_numeric_h_
