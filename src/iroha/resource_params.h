// -*- C++ -*-
#ifndef _iroha_resource_params_h_
#define _iroha_resource_params_h_

#include "iroha/common.h"

namespace iroha {

namespace resource {
class ResourceParamValueSet;

const char kResetPolarity[] = "reset_polarity";
const char kExtInputPort[] = "input";
const char kExtOutputPort[] = "output";
const char kExtIOWidth[] = "width";
const char kEmbeddedModule[] = "embedded_module";
const char kEmbeddedModuleFile[] = "embedded_module_file";
}  // namespace resource

class ResourceParams {
public:
  ResourceParams();
  ~ResourceParams();

  bool GetResetPolarity() const;
  void SetResetPolarity(bool p);

  void SetExtInputPort(const string &input, int width);
  void GetExtInputPort(string *name, int *width);
  void SetExtOutputPort(const string &output, int width);
  void GetExtOutputPort(string *name, int *width);

  void SetEmbeddedModuleName(const string &mod, const string &fn);

private:
  resource::ResourceParamValueSet *values_;
};

}  // namespace iroha

#endif  // _iroha_resource_params_h_
