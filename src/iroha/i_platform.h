// -*- C++ -*-
#ifndef _iroha_i_platform_h_
#define _iroha_i_platform_h_

#include "iroha/common.h"

namespace iroha {

namespace platform {

class Condition {
public:
  Condition(Definition *definition);
  ~Condition();

  Definition *GetDefinition();

private:
  Definition *definition_;
};

class Value {
public:
  Value(Definition *definition);
  ~Value();

  Definition *GetDefinition();

private:
  Definition *definition_;
};

class Definition {
public:
  Definition(IPlatform *platform);
  ~Definition();

  IPlatform *GetPlatform();

  Condition *condition_;
  Value *value_;

private:
  IPlatform *platform_;
};

}  // namespace platform

class IPlatform {
public:
  IPlatform(IDesign *design);
  ~IPlatform();

  ObjectPool *GetObjectPool();

  vector<platform::Definition *> defs_;

private:
  ObjectPool *objects_;
  IDesign *design_;
};

}  // namespace iroha

#endif  // _iroha_i_platform_h_

