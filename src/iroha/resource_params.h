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
const char kMaxDelayPs[] = "MAX-DELAY-PS";
const char kPlatformFamily[] = "PLATFORM-FAMILY";
const char kPlatformName[] = "PLATFORM-NAME";
const char kModuleNamePrefix[] = "MODULE-NAME-PREFIX";
const char kPortNamePrefix[] = "PORT-NAME-PREFIX";
const char kResetName[] = "RESET-NAME";
const char kAddrWidth[] = "ADDR-WIDTH";
const char kExtInputPort[] = "INPUT";
const char kExtOutputPort[] = "OUTPUT";
const char kExtIOWidth[] = "WIDTH";
const char kInitialValue[] = "INITIAL-VALUE";
const char kDefaultOutputValue[] = "DEFAULT-VALUE";
const char kEmbeddedModule[] = "EMBEDDED-MODULE";
const char kEmbeddedModuleFile[] = "EMBEDDED-MODULE-FILE";
const char kEmbeddedModuleClk[] = "EMBEDDED-MODULE-CLOCK";
const char kEmbeddedModuleReset[] = "EMBEDDED-MODULE-RESET";
const char kEmbeddedModuleInputs[] = "EMBEDDED-MODULE-INPUTS";
const char kEmbeddedModuleOutputs[] = "EMBEDDED-MODULE-OUTPUTS";
const char kSramPortIndex[] = "SRAM-PORT-INDEX";
const char kExtTaskName[] = "EXT-TASK";
const char kDistance[] = "DISTANCE";
const char kWenSuffix[] = "WEN";
const char kNotifySuffix[] = "NOTIFY";
const char kPutSuffix[] = "PUT";
}  // namespace resource

class ResourceParams {
public:
  ResourceParams();
  ~ResourceParams();

  void Merge(ResourceParams *src_params);

  vector<string> GetParamKeys() const;
  vector<string> GetValues(const string &key) const;
  void SetValues(const string &key, const vector<string> &values);

  // Accessors for convenience.

  string GetModuleNamePrefix() const;
  void SetModuleNamePrefix(const string &name);

  string GetPortNamePrefix() const;
  void SetPortNamePrefix(const string &name);

  string GetSramPortIndex() const;
  void SetSramPortIndex(const string &idx);

  string GetMappedName() const;
  void SetMappedName(const string &name);

  bool GetResetPolarity() const;
  void SetResetPolarity(bool p);
  bool HasResetPolarity() const;

  string GetPlatformFamily() const;
  void SetPlatformFamily(const string &name);

  string GetPlatformName() const;
  void SetPlatformName(const string &name);

  int GetMaxDelayPs() const;
  void SetMaxDelayPs(int ps);

  string GetResetName() const;
  void SetResetName(const string &name);

  int GetAddrWidth() const;
  void SetAddrWidth(int width);

  void SetExtInputPort(const string &input, int width);
  void GetExtInputPort(string *name, int *width);
  void SetExtOutputPort(const string &output, int width);
  void GetExtOutputPort(string *name, int *width);

  bool GetInitialValue(int *value) const;
  void SetInitialValue(int value);
  // DEFAULT-VALUE
  bool GetDefaultValue(int *value) const;
  void SetDefaultValue(int value);
  int GetWidth();
  void SetWidth(int w);

  void SetEmbeddedModuleName(const string &mod, const string &fn);
  string GetEmbeddedModuleName() const;
  string GetEmbeddedModuleFileName() const;
  string GetEmbeddedModuleClk() const;
  string GetEmbeddedModuleReset() const;
  void SetEmbeddedModuleIO(bool is_output, const vector<string> &ports);
  vector<string> GetEmbeddedModuleIO(bool is_output);

  string GetExtTaskName() const;
  void SetExtTaskName(const string &name);

  int GetDistance() const;
  void SetDistance(int distance);

  // for shared-reg-ext-writer.
  void SetWenSuffix(const string &wen);
  string GetWenSuffix() const;
  void SetNotifySuffix(const string &notify);
  string GetNotifySuffix() const;
  void SetPutSuffix(const string &put);
  string GetPutSuffix();

private:
  resource::ResourceParamValueSet *values_;
};

}  // namespace iroha

#endif  // _iroha_resource_params_h_
