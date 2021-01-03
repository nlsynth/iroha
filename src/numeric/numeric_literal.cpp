#include "numeric/numeric_literal.h"

#include "iroha/base/util.h"
#include "numeric/numeric.h"
#include "numeric/numeric_op.h"

#include <stdlib.h>

using std::string;

namespace iroha {

NumericLiteral NumericLiteral::Parse(const std::string &token) {
  string tt = RemoveUnderscore(token);
  const char *t = tt.c_str();
  iroha::NumericLiteral nl;
  nl.has_error = false;
  nl.width = - 1;
  if (t[0] == '0') {
    if (t[1] == 'x') {
      uint64_t num;
      if (sscanf(t, "%llx", (long long unsigned int *)&num) != 1) {
	nl.has_error = true;
      }
      nl.value = num;
      return nl;
    }
    if (t[1] == 'b') {
      nl.value = Parse0b(tt);
      nl.width = tt.size() - 2;
      if (nl.width >= 64) {
	nl.has_error = true;
      }
      return nl;
    }
  }
  try {
    nl.value = std::stoull(tt, nullptr, 10);
  } catch (...) {
    nl.value = 0;
    nl.has_error = true;
  }
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

void NumericLiteral::ToNumeric(NumericManager *mgr, Numeric *numeric) {
  iroha::Op::MakeConst0(value, numeric->GetMutableValue());
  if (width > -1) {
    numeric->type_.SetWidth(width);
  }
  iroha::Numeric::MayExpandStorage(mgr, numeric);
}

std::string NumericLiteral::RemoveUnderscore(const std::string &s) {
  vector<string> c;
  Util::SplitStringUsing(s, "_", &c);
  return Util::Join(c, "");
}

}  // namespace iroha
