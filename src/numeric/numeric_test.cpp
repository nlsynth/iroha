#include "numeric/numeric_type.h"
#include "numeric/numeric_op.h"

#include <iostream>

using namespace iroha;
using namespace std;

int main(int argc, char **argv) {
  cout << "sizeof(width)=" << sizeof(NumericWidth) << "\n";
  cout << "sizeof(numeric)=" << sizeof(Numeric) << "\n";
  return 0;
}
