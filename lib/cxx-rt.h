// -*- C++ -*-
#ifndef _cxx_rt_h_
#define _cxx_rt_h_

#include <iostream>
#include <vector>

using std::cout;
using std::vector;

namespace iroha_rt {

class TableBase {
public:
  virtual void dispatch() = 0;
};

class ModuleBase {
public:
  void dispatch() {
    for (TableBase *t : tabs_) {
      t->dispatch();
    }
  }
  vector<TableBase *> tabs_;
};

class Runner {
public:
  Runner(ModuleBase *mod) : mod_(mod) {
  }
  void Run() {
    for (int i = 0; i < 100; ++i) {
      mod_->dispatch();
    }
  }

private:
  ModuleBase *mod_;
};

}  // namespace iroha_rt

#endif  // cxx_rt_h_
