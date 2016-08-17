// -*- C++ -*-
#ifndef _cxx_rt_h_
#define _cxx_rt_h_

namespace iroha_rt {

class ModuleBase {
public:
};

class TableBase {
public:
};

class Runner {
public:
  Runner(ModuleBase &mod) : mod_(mod) {
  }
  void Run() {
  }

private:
  ModuleBase &mod_;
};

}  // namespace iroha_rt

#endif  // cxx_rt_h_

