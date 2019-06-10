#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"
#include "numeric/wide_op.h"

#include <assert.h>
#include <iostream>

using namespace iroha;
using namespace std;

namespace {

class Tool {
public:
  static uint64_t GetValue(const Numeric &n, int idx) {
    assert(n.type_.IsWide());
    return n.GetArray().value_[idx];
  }
  static void AssertEq(uint64_t e, uint64_t a) {
    assert(e == a);
  }
};

}  // namespace

void shift() {
  cout << "Shift\n";
  Numeric n;
  n.type_.SetWidth(512);
  Op::Clear(&n);
  n.SetValue0(0xfffffffffffffff0ULL);
  cout << "n=" << n.Format() << "\n";
  Tool::AssertEq(0xfffffffffffffff0UL, n.GetValue0());
  Tool::AssertEq(0, Tool::GetValue(n, 1));

  Numeric m;
  m.type_.SetWidth(512);
  Op::Clear(&m);
  // Left
  cout << "Left Shift\n";
  WideOp::Shift(n, 1, true, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(1, Tool::GetValue(m, 1));

  WideOp::Shift(n, 64, true, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(0, Tool::GetValue(m, 0));
  Tool::AssertEq(Tool::GetValue(n, 0), Tool::GetValue(m, 1));

  WideOp::Shift(n, 65, true, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(1, Tool::GetValue(m, 2));

  WideOp::Shift(n, 385, true, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(1, Tool::GetValue(m, 7));

  // Right
  cout << "Right Shift\n";
  n = m;
  WideOp::Shift(n, 0, false, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(1, Tool::GetValue(m, 7));

  WideOp::Shift(n, 64, false, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(1, Tool::GetValue(m, 6));

  WideOp::Shift(n, 65, false, &m);
  cout << "m=" << m.Format() << "\n";
  Tool::AssertEq(0xfffffffffffffff0ULL, Tool::GetValue(m, 5));
}

void concat() {
  cout << "concat\n";
  Numeric m, n;
  m.type_.SetWidth(64);
  m.SetValue0(0x5555555555555550ULL);
  n.type_.SetWidth(63);
  n.SetValue0(0x5555555555555550ULL);
  Numeric r;
  Op::Concat(m, n, &r);
  cout << "r=" << r.Format() << "\n";
}

void fixup() {
  cout << "fixup\n";
  Numeric n;
  n.type_.SetWidth(68);
  n.SetValue0(0xfffffffffffffff0ULL);
  Numeric m;
  WideOp::Shift(n, 16, true, &m);
  m.type_ = n.type_;
  cout << "m=" << m.Format() << "\n";
  Op::FixupWidth(m.type_, &m);
  cout << "m=" << m.Format() << "\n";
}

int main(int argc, char **argv) {
  cout << "sizeof(width)=" << sizeof(NumericWidth) << "\n";
  cout << "sizeof(numeric)=" << sizeof(Numeric) << "\n";

  Numeric n;
  n.type_.SetWidth(64);
  n.type_.SetIsSigned(true);
  n.SetValue0(0xfffffffffffffff0ULL);
  cout << "n=" << n.Format() << "\n";
  n.SetValue0(15);
  cout << "n=" << n.Format() << "\n";

  Numeric w;
  w.type_.SetWidth(512);
  Op::Clear(&w);
  cout << "w=" << w.Format() << "\n";

  shift();
  concat();
  fixup();

  return 0;
}
