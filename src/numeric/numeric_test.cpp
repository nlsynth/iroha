#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"

#include <iostream>

using namespace iroha;
using namespace std;

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
  return 0;
}
