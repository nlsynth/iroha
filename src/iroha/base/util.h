// -*- C++ -*-
#ifndef _iroha_base_util_h_
#define _iroha_base_util_h_

#include <string>
#include <vector>

using namespace std;

namespace iroha {

class Util {
 public:
  static string Itoa(int i);
  static int Atoi(const string &a);
  static uint64_t AtoULL(const string &a);
  static string ULLtoA(uint64_t u);
  static int Log2(uint64_t u);
  static bool IsInteger(const string &a);
  static void SplitStringUsing(const string &str, const char *delim,
                               vector<string> *output);
  static string ToLower(const string &s);
  static string Join(const vector<string> &v, const string &sep);
  static bool EndsWith(const string &s, const string &suffix);
};

}  // namespace iroha

#endif  // _iroha_base_util_h_
