// -*- C++ -*-
#ifndef _iroha_common_h_
#define _iroha_common_h_

#include "iroha/util.h"

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

namespace iroha {
class IArray;
class IArrayImage;
class IDesign;
class IInsn;
class IModule;
class IPlatform;
class IProfile;
class IRegister;
class IResource;
class IResourceClass;
class IState;
class ITable;
class IValueType;
class Numeric;
class ObjectPool;
class ResourceParams;

namespace platform {
class DefNode;
class Definition;
class PlatformDB;
}  // namespace platform

}  // namespace iroha

#endif  // _iroha_common_h_
