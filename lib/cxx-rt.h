// -*- C++ -*-
#ifndef _cxx_rt_h_
#define _cxx_rt_h_

#include <iostream>
#include <map>
#include <vector>

using std::cout;
using std::map;
using std::vector;

namespace iroha_rt {

class Memory {
public:
  static const int kPageSize = 1024;
  struct Page {
    Page() {
      for (int i = 0; i < kPageSize; ++i) {
	data[kPageSize] = 0;
      }
    }
    int data[kPageSize];
  };
  map<int, Page *> pages_;
  int read_addr_;

  Memory() : read_addr_(0) {
  }

  ~Memory() {
    for (auto it : pages_) {
      delete it.second;
    }
  }

  void ReadAddr(int addr) {
    read_addr_ = addr;
  }

  int ReadData() {
    Page *p = GetPage(read_addr_);
    return p->data[GetOffset(read_addr_)];
  }

  void Write(int addr, int data) {
    Page *p = GetPage(addr);
    p->data[GetOffset(addr)] = data;
  }

  Page *GetPage(int addr) {
    int index = addr / (4  * kPageSize);
    auto it = pages_.find(index);
    if (it != pages_.end()) {
      return it->second;
    }
    Page *p = new Page();
    pages_[index] = p;
    return p;
  }

  int GetOffset(int addr) {
    return (addr % (4  * kPageSize)) / 4;
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
