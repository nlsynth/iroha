#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"
#include "numeric/wide_op.h"

#include <iostream>

using namespace iroha;
using namespace std;

void shift() {
  Numeric n;
  n.type_.SetWidth(512);
  Op::Clear(&n);
  n.SetValue(0xfffffffffffffff0ULL);
  cout << "n=" << n.Format() << "\n";

  Numeric m;
  m.type_.SetWidth(512);
  Op::Clear(&m);
  WideOp::Shift(n, 1, true, &m);
  cout << "m=" << m.Format() << "\n";
  WideOp::Shift(n, 64, true, &m);
  cout << "m=" << m.Format() << "\n";
  WideOp::Shift(n, 65, true, &m);
  cout << "m=" << m.Format() << "\n";
}

int main(int argc, char **argv) {
  cout << "sizeof(width)=" << sizeof(NumericWidth) << "\n";
  cout << "sizeof(numeric)=" << sizeof(Numeric) << "\n";

  Numeric n;
  n.type_.SetWidth(64);
  n.type_.SetIsSigned(true);
  n.SetValue(0xfffffffffffffff0ULL);
  cout << "n=" << n.Format() << "\n";
  n.SetValue(15);
  cout << "n=" << n.Format() << "\n";

  Numeric w;
  w.type_.SetWidth(512);
  Op::Clear(&w);
  cout << "w=" << w.Format() << "\n";

  shift();

  return 0;
}
