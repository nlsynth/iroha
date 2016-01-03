// -*- C++ -*-
#ifndef _iroha_common_h_
#define _iroha_common_h_

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

namespace iroha {
class IArray;
class IChannel;
class IDesign;
class IInsn;
class IModule;
class IRegister;
class IResource;
class IResourceClass;
class IState;
class ITable;
class IValue;
class IValueType;
class ObjectPool;
class ResourceParams;

class Util {
public:
  static string Itoa(int i);
  static int Atoi(const string &a);
};

}  // namespace iroha

#endif  // _iroha_common_h_
