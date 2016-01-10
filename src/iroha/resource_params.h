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

const char kMappedName[] = "mapped_name";
const char kResetPolarity[] = "reset_polarity";
const char kExtInputPort[] = "input";
const char kExtOutputPort[] = "output";
const char kExtIOWidth[] = "width";
const char kEmbeddedModule[] = "embedded_module";
const char kEmbeddedModuleFile[] = "embedded_module_file";
const char kEmbeddedModuleClk[] = "embedded_module_clk";
const char kEmbeddedModuleReset[] = "embedded_module_reset";
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
