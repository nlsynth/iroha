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
  static void SetImportPaths(const vector<string> &paths);
  static void SplitStringUsing(const string &str, const char *delim,
			       vector<string> *output);
  static string ToLower(const string &s);
  static string Join(const vector<string> &v, const string &sep);
  static istream *OpenFile(const string &s);

  static vector<string> import_paths_;
};

}  // namespace iroha

#endif  // _iroha_common_h_
