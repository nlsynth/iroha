// -*- C++ -*-
#ifndef _iroha_i_platform_h_
#define _iroha_i_platform_h_

#include "iroha/common.h"

namespace iroha {

namespace platform {

class DefNode {
public:
  DefNode(Definition *definition);
  ~DefNode();

  Definition *GetDefinition();
  const string &GetHead();

  bool is_atom_;
  int num_;
  string str_;
  vector<DefNode *> nodes_;

private:
  Definition *definition_;
};

class Definition {
public:
  Definition(IPlatform *platform);
  ~Definition();

  IPlatform *GetPlatform();

  DefNode *condition_;
  DefNode *value_;

private:
  IPlatform *platform_;
};

}  // namespace platform

class IPlatform {
public:
  IPlatform(IDesign *design);
  ~IPlatform();

  ObjectPool *GetObjectPool();
  IDesign *GetDesign();
  void SetDesign(IDesign *design);
  const string &GetName() const;
  void SetName(const string &name);

  vector<platform::Definition *> defs_;

private:
  IDesign *design_;
  ObjectPool *objects_;
  string name_;
};

}  // namespace iroha

#endif  // _iroha_i_platform_h_

