#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"
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
    assert(n.type_.IsWide());
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
  TestUtil::AssertEq(0xfffffffffffffff0UL, n.GetValue0());
  TestUtil::AssertEq(0, Tool::GetValue(n, 1));

  Numeric m;
  m.type_.SetWidth(512);
  Op::Clear(m.type_, m.GetMutableArray());
  // Left
  cout << "Left Shift\n";
  WideOp::Shift(n.GetArray(), 1, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(1, Tool::GetValue(m, 1));

  WideOp::Shift(n.GetArray(), 64, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(0, Tool::GetValue(m, 0));
  TestUtil::AssertEq(Tool::GetValue(n, 0), Tool::GetValue(m, 1));

  WideOp::Shift(n.GetArray(), 65, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(1, Tool::GetValue(m, 2));

  WideOp::Shift(n.GetArray(), 385, true, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(1, Tool::GetValue(m, 7));

  // Right
  cout << "Right Shift\n";
  n = m;
  WideOp::Shift(n.GetArray(), 0, false, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(1, Tool::GetValue(m, 7));

  WideOp::Shift(n.GetArray(), 64, false, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(1, Tool::GetValue(m, 6));

  WideOp::Shift(n.GetArray(), 65, false, m.GetMutableArray());
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(0xfffffffffffffff0ULL, Tool::GetValue(m, 5));
}

void Concat() {
  TEST_CASE("Concat");
  Numeric m, n;
  m.type_.SetWidth(64);
  m.SetValue0(0x5555555555555550ULL);
  n.type_.SetWidth(63);
  n.SetValue0(0x5555555555555550ULL);
  Numeric r;
  Op::Concat(m.GetArray(), m.type_, n.GetArray(), n.type_,
	     r.GetMutableArray(), &r.type_);
  cout << "r=" << r.Format() << "\n";
  TestUtil::Assert(r.type_.GetWidth() == 127);
  TestUtil::AssertEq(0x5555555555555550ULL, Tool::GetValue(r, 0));
  TestUtil::AssertEq(0x2aaaaaaaaaaaaaa8ULL, Tool::GetValue(r, 1));
}

void Fixup() {
  TEST_CASE("Fixup");
  Numeric n;
  n.type_.SetWidth(68);
  n.SetValue0(0xfffffffffffffff0ULL);
  Numeric m;
  WideOp::Shift(n.GetArray(), 16, true, m.GetMutableArray());
  m.type_ = n.type_;
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(65535, Tool::GetValue(m, 1));

  Op::FixupWidth(m.type_, &m);
  cout << "m=" << m.Format() << "\n";
  TestUtil::AssertEq(15, Tool::GetValue(m, 1));
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
  TestUtil::Assert(Op::IsZero(w.type_, w.GetArray()));
}

void ExtraWide() {
  TEST_CASE("ExtraWide");
  Numeric n;
  Numeric::Clear(n.type_, n.GetMutableArray());
  n.SetValue0(2);
  n.type_.SetWidth(1024);
  Numeric::MayExpandStorage(nullptr, &n);
  TestUtil::Assert(n.GetValue0() == 2);

  Numeric nn;
  nn.type_.SetWidth(768);
  Numeric::MayPopulateStorage(nn.type_, nullptr, nn.GetMutableArray());

  Op::SelectBits(n.GetArray(), n.type_,
		 768, 1, nn.GetMutableArray(), nullptr);
  TestUtil::Assert(nn.GetValue0() == 1);
}

int main(int argc, char **argv) {
  cout << "sizeof(width)=" << sizeof(NumericWidth) << "\n";
  cout << "sizeof(numeric)=" << sizeof(Numeric) << "\n";

  Basic();
  Shift();
  Concat();
  Fixup();
  ExtraWide();

  return 0;
}
