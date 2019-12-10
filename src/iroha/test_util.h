// -*- C++ -*-
#ifndef _iroha_test_util_h_
#define _iroha_test_util_h_

#include <assert.h>
#include <stdint.h>
#include <iostream>

class TestUtil {
public:
  static void AssertEq(uint64_t e, uint64_t a, const char *fn, int line) {
    if (e != a) {
      std::cerr << fn << ":" << line << " assertion failed. "
		<< " expected: " << e << " got: " << a;
      abort();
    }
  }
  static void Assert(bool b, const char *fn, int line) {
    if (!b) {
      std::cerr << fn << ":" << line << " assertion failed.";
      abort();
    }
  }
  static void PrintTestName(const char *c) {
    std::cout << "TestCase: " << c << "\n";
  }
};

#define TEST_CASE(x) do { TestUtil::PrintTestName(x); } while (false)

#define ASSERT_EQ(x, y) do { TestUtil::AssertEq(x, y, __FILE__, __LINE__); } while (false)
#define ASSERT(x) do { TestUtil::Assert(x, __FILE__, __LINE__); } while (false)

#endif  // _iroha_test_util_h_
