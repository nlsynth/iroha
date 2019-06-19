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

class Numeric {
public:
  Numeric();

  // Get/SetValue0 are convenient interface for small values.
  uint64_t GetValue0() const {
    if (type_.IsExtraWide()) {
      return value_.extra_wide_value_->value_[0];
    } else {
      return value_.value_[0];
    }
  }
  void SetValue0(uint64_t value);
  const NumericValue &GetArray() const {
    return value_;
  }
  NumericValue *GetMutableArray() {
    return &value_;
  }
  std::string Format() const;
  static std::string Format(const NumericWidth &w, const NumericValue &val);

  // Manages storage for big number.
  static NumericManager *DefaultManager();
  static NumericManager *CreateManager();
  static void DeleteManager(NumericManager *mgr);
  static void CopyValue(const Numeric &src, NumericManager *mgr, Numeric *dst);
  static void CopyValueWithWidth(const NumericValue &src_value,
				 const NumericWidth &src_width,
				 const NumericWidth &dst_width,
				 NumericManager *mgr,
				 NumericValue *value);
  static void MayPopulateStorage(const NumericWidth &width, NumericManager *mgr,
				 Numeric *n);
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

#endif  // _numeric_numeric_type_h_
