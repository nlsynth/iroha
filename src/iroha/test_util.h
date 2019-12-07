// -*- C++ -*-
#ifndef _iroha_test_util_h_
#define _iroha_test_util_h_

#include <assert.h>
#include <stdint.h>
#include <iostream>

class TestUtil {
public:
  static void AssertEq(uint64_t e, uint64_t a) {
    assert(e == a);
  }
  static void Assert(bool b) {
    assert(b);
  }
  static void PrintTestName(const char *c) {
    std::cout << "TestCase: " << c << "\n";
  }
};

#define TEST_CASE(x) do { TestUtil::PrintTestName(x); } while (false)

#endif  // _iroha_test_util_h_
