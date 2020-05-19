#include "numeric/numeric_literal.h"

#include <stdlib.h>

namespace iroha {

NumericLiteral NumericLiteral::Parse(const std::string &token) {
  const char *t = token.c_str();
  iroha::NumericLiteral nl;
  nl.width = -1;
  if (t[0] == '0') {
    if (t[1] == 'x') {
      uint64_t num;
      sscanf(t, "%llx", (long long unsigned int *)&num);
      nl.value = num;
      return nl;
    }
    if (t[1] == 'b') {
      nl.value = Parse0b(token);
      nl.width = token.size() - 2;
      return nl;
    }
  }
  nl.value = atoll(t);
  return nl;
}

uint64_t NumericLiteral::Parse0b(const std::string &token) {
  uint64_t u = 0;
  const char *t = token.c_str();
  for (int i = 2; i < token.size(); ++i) {
    u <<= 1;
    if (t[i] == '1') {
      u += 1;
    }
  }
  return u;
}

}  // namespace iroha
