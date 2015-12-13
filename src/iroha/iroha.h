// -*- C++ -*-
//
// Main APIs and structures are defined (or included from this file).
//
#ifndef _iroha_iroha_h_
#define _iroha_iroha_h_

#include "iroha/common.h"
#include "iroha/writer_api.h"

namespace iroha {

class IState {
public:
};
  
class ITable {
public:
  vector<IState *> states_;
};

class IModule {
public:
  vector<ITable *> tables_;
};

class IDesign {
public:
  vector<IModule *> modules_;
};

// High level APIs and factory methods.
class Iroha {
public:
  static void Init();
  static IDesign *ReadDesignFromFile(const string &fn);
  static WriterAPI *CreateWriter(const IDesign *design);
};

}  // namespace iroha

#endif  // _iroha_iroha_h_
