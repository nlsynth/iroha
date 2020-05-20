#include "numeric/numeric.h"
#include "numeric/numeric_literal.h"
#include "numeric/numeric_op.h"
#include "numeric/numeric_value.h"
#include "numeric/numeric_width.h"
#include "numeric/wide_op.h"

#include "iroha/test_util.h"

#include <assert.h>
#include <iostream>

using namespace iroha;
using namespace std;

namespace {

class Tool {
public:
  static uint64_t GetValue(const Numeric &n, int idx) {
    ASSERT(n.type_.IsWide());
    return n.GetArray().value_[idx];
  }
};

}  // namespace

void Shift() {
  TEST_CASE("Shift");
  Numeric n;
  n.type_.SetWidth(512);
  Op::Clear(n.type_, n.GetMutableArray());
  n.SetValue0(0xfffffffffffffff0ULL);
  cout << "n=" << n.Format() << "\n";
  ASSERT_EQ(0xfffffffffffffff0UL, n.GetValue0());
  ASSERT_EQ(0, Tool::GetValue(n, 1));

  Numeric m;
  m.type_.SetWidth(512);
  Op::Clear(m.type_, m.GetMutableArray());
  // Left
  cout << "Left Shift\n";
  WideOp::Shift(n.GetArray(), 1, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(1, Tool::GetValue(m, 1));

  WideOp::Shift(n.GetArray(), 64, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(0, Tool::GetValue(m, 0));
  ASSERT_EQ(Tool::GetValue(n, 0), Tool::GetValue(m, 1));

  WideOp::Shift(n.GetArray(), 65, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(1, Tool::GetValue(m, 2));

  WideOp::Shift(n.GetArray(), 385, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(1, Tool::GetValue(m, 7));

  // Right
  cout << "Right Shift\n";
  n = m;
  WideOp::Shift(n.GetArray(), 0, false, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(1, Tool::GetValue(m, 7));

  WideOp::Shift(n.GetArray(), 64, false, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(1, Tool::GetValue(m, 6));

  WideOp::Shift(n.GetArray(), 65, false, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(0xfffffffffffffff0ULL, Tool::GetValue(m, 5));
}

void Concat() {
  TEST_CASE("Concat");
  Numeric m, n;
  Numeric::Clear(m.type_, m.GetMutableArray());
  m.type_.SetWidth(64);
  m.SetValue0(0x5555555555555550ULL);
  Numeric::Clear(n.type_, n.GetMutableArray());
  n.type_.SetWidth(63);
  n.SetValue0(0x5555555555555550ULL);
  Numeric r;
  Op::Concat(m.GetArray(), m.type_, n.GetArray(), n.type_,
	     r.GetMutableArray(), &r.type_);
  cout << "r=" << r.Format() << "\n";
  ASSERT(r.type_.GetWidth() == 127);
  ASSERT_EQ(0x5555555555555550ULL, Tool::GetValue(r, 0));
  ASSERT_EQ(0x2aaaaaaaaaaaaaa8ULL, Tool::GetValue(r, 1));
}

void Fixup() {
  TEST_CASE("Fixup");
  Numeric n;
  Numeric::Clear(n.type_, n.GetMutableArray());
  n.type_.SetWidth(68);
  n.SetValue0(0xfffffffffffffff0ULL);
  Numeric m;
  WideOp::Shift(n.GetArray(), 16, true, m.GetMutableArray());
  m.type_ = n.type_;
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(65535, Tool::GetValue(m, 1));

  Op::FixupValueWidth(m.type_, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  ASSERT_EQ(15, Tool::GetValue(m, 1));
}

void Basic() {
  TEST_CASE("Basic");
  Numeric n;
  n.type_.SetWidth(64);
  n.type_.SetIsSigned(true);
  n.SetValue0(0xfffffffffffffff0ULL);
  cout << "n=" << n.Format() << "\n";
  n.SetValue0(15);
  cout << "n=" << n.Format() << "\n";

  Numeric w;
  w.type_.SetWidth(512);
  Op::Clear(w.type_, w.GetMutableArray());
  cout << "w=" << w.Format() << "\n";
  ASSERT(Op::IsZero(w.type_, w.GetArray()));
}

void ExtraWide() {
  TEST_CASE("ExtraWide");
  Numeric n;
  Numeric::Clear(n.type_, n.GetMutableArray());
  n.SetValue0(2);
  n.type_.SetWidth(1024);
  Numeric::MayExpandStorage(nullptr, &n);
  ASSERT(n.GetValue0() == 2);

  Numeric nn;
  nn.type_.SetWidth(768);
  Numeric::MayPopulateStorage(nn.type_, nullptr, nn.GetMutableArray());

  Op::SelectBits(n.GetArray(), n.type_,
		 768, 1, nn.GetMutableArray(), nullptr);
  ASSERT(nn.GetValue0() == 1);

  Numeric m;
  Op::SelectBits(n.GetArray(), n.type_,
		 512, 1, m.GetMutableArray(), nullptr);
  ASSERT(m.GetValue0() == 1);
}

void Set() {
  TEST_CASE("Set");
  Numeric s;
  Numeric::Clear(s.type_, s.GetMutableArray());
  s.SetValue0(1);
  s.type_.SetWidth(64);

  Numeric d;
  d.type_.SetWidth(64);
  Op::Set(s.type_, s.GetArray(), d.type_, d.GetMutableArray());
  ASSERT(d.GetValue0() == 1);
}

void Eq() {
  TEST_CASE("Eq");
  Numeric n1, n2;
  n1.type_.SetWidth(8);
  n1.SetValue0(0x105);
  n2.type_.SetWidth(8);
  n2.SetValue0(0x05);
  ASSERT(Op::Eq(n1.type_, n1.GetArray(), n2.GetArray()));
  n1.type_.SetWidth(9);
  ASSERT(!Op::Eq(n1.type_, n1.GetArray(), n2.GetArray()));
}

void Literal() {
  NumericLiteral nl;
  nl = nl.Parse("123");
  ASSERT(nl.value == 123);
  ASSERT(nl.width < 0);
  nl = nl.Parse("0b1010");
  ASSERT(nl.value == 10);
  ASSERT(nl.width == 4);
  nl = nl.Parse("0xa0");
  ASSERT(nl.value == 160);
  ASSERT(nl.width < 0);
}

int main(int argc, char **argv) {
  cout << "sizeof(width)=" << sizeof(NumericWidth) << "\n";
  cout << "sizeof(numeric)=" << sizeof(Numeric) << "\n";

  Basic();
  Shift();
  Concat();
  Fixup();
  ExtraWide();
  Set();
  Eq();
  Literal();

  return 0;
}
