// -*- C++ -*-
#ifndef _cxx_rt_h_
#define _cxx_rt_h_

#include <iostream>
#include <vector>

using std::cout;
using std::vector;

namespace iroha_rt {

class Memory {
public:
  void ReadAddr(int addr) {
  }
  int ReadData() {
    return 0;
  }
  void Write(int addr, int data) {
  }
};

class TableBase {
public:
  void SetMemory(Memory *mem) {
    mem_ = mem;
  }
  virtual void dispatch() = 0;

protected:
  Memory *mem_;
};

class ModuleBase {
public:
  void dispatch() {
    for (TableBase *t : tabs_) {
      t->dispatch();
    }
  }
  void SetMemory(Memory *mem) {
    for (TableBase *t : tabs_) {
      t->SetMemory(mem);
    }
  }
  vector<TableBase *> tabs_;
};

class Runner {
public:
  Runner(ModuleBase *mod) : mod_(mod) {
    mod->SetMemory(&mem_);
  }
  void Run() {
    for (int i = 0; i < 100; ++i) {
      mod_->dispatch();
    }
  }

private:
  ModuleBase *mod_;
  Memory mem_;
};

}  // namespace iroha_rt

#endif  // cxx_rt_h_
