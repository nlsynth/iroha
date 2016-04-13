// -*- C++ -*-
//
// Parameters of various kinds of resources like design, external I/O,
// embedded module and so on.
//
#ifndef _iroha_resource_params_h_
#define _iroha_resource_params_h_

#include "iroha/common.h"

namespace iroha {

namespace resource {
class ResourceParamValueSet;

const char kMappedName[] = "MAPPED-NAME";
const char kResetPolarity[] = "RESET-POLARITY";
const char kResetName[] = "RESET-NAME";
const char kExtInputPort[] = "INPUT";
const char kExtOutputPort[] = "OUTPUT";
const char kExtIOWidth[] = "WIDTH";
const char kEmbeddedModule[] = "EMBEDDED-MODULE";
const char kEmbeddedModuleFile[] = "EMBEDDED-MODULE-FILE";
const char kEmbeddedModuleClk[] = "EMBEDDED-MODULE-CLK";
const char kEmbeddedModuleReset[] = "EMBEDDED-MODULE-RESET";
}  // namespace resource

class ResourceParams {
public:
  ResourceParams();
  ~ResourceParams();

  vector<string> GetParamKeys() const;
  vector<string> GetValues(const string &key) const;
  void SetValues(const string &key, const vector<string> &values);

  string GetMappedName() const;
  void SetMappedName(const string &name);

  bool GetResetPolarity() const;
  void SetResetPolarity(bool p);
  bool HasResetPolarity() const;

  string GetResetName() const;
  void SetResetName(const string &name);

  void SetExtInputPort(const string &input, int width);
  void GetExtInputPort(string *name, int *width);
  void SetExtOutputPort(const string &output, int width);
  void GetExtOutputPort(string *name, int *width);

  void SetEmbeddedModuleName(const string &mod, const string &fn);
  string GetEmbeddedModuleName() const;
  string GetEmbeddedModuleFileName() const;
  string GetEmbeddedModuleClk() const;
  string GetEmbeddedModuleReset() const;

private:
  resource::ResourceParamValueSet *values_;
};

}  // namespace iroha

#endif  // _iroha_resource_params_h_
