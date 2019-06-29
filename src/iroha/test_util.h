// -*- C++ -*-
#ifndef _iroha_test_util_h_
#define _iroha_test_util_h_

#include <assert.h>
#include <stdint.h>

class TestUtil {
public:
  static void AssertEq(uint64_t e, uint64_t a) {
    assert(e == a);
  }
  static void Assert(bool b) {
    assert(b);
  }
};

#endif  // _iroha_test_util_h_
