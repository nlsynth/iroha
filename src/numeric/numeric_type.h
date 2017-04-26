// -*- C++ -*-
#ifndef _numeric_numeric_type_h_
#define _numeric_numeric_type_h_

#include <cstdint>

namespace iroha {

class IValueType {
public:
  IValueType();

  // Width 0 means a scalar value e.g. 'reg v;'.
  int GetWidth() const;
  void SetWidth(int width);

  bool IsSigned() const;
  void SetIsSigned(bool is_signed);

private:
  int width_;
  bool is_signed_;
};

class Numeric {
public:
  Numeric();
  uint64_t value_;
  IValueType type_;
};

}  // namespace iroha

#endif  // _numeric_numeric_type_h_
