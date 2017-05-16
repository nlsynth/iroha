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
const char kModuleNamePrefix[] = "MODULE-NAME-PREFIX";
const char kPortNamePrefix[] = "PORT-NAME-PREFIX";
const char kResetName[] = "RESET-NAME";
const char kAddrWidth[] = "ADDR-WIDTH";
const char kExtInputPort[] = "INPUT";
const char kExtOutputPort[] = "OUTPUT";
const char kExtIOWidth[] = "WIDTH";
const char kDefaultOutputValue[] = "DEFAULT-VALUE";
const char kEmbeddedModule[] = "EMBEDDED-MODULE";
const char kEmbeddedModuleFile[] = "EMBEDDED-MODULE-FILE";
const char kEmbeddedModuleClk[] = "EMBEDDED-MODULE-CLOCK";
const char kEmbeddedModuleReset[] = "EMBEDDED-MODULE-RESET";
const char kEmbeddedModuleReq[] = "EMBEDDED-MODULE-REQ";
const char kEmbeddedModuleAck[] = "EMBEDDED-MODULE-ACK";
const char kEmbeddedModuleArgs[] = "EMBEDDED-MODULE-ARGS";
const char kChannelDataPort[] = "CHANNEL-DATA";
const char kChannelEnPort[] = "CHANNEL-EN";
const char kChannelAckPort[] = "CHANNEL-ACK";
const char kExtTaskName[] = "EXT-TASK";
}  // namespace resource

class ResourceParams {
public:
  ResourceParams();
  ~ResourceParams();

  vector<string> GetParamKeys() const;
  vector<string> GetValues(const string &key) const;
  void SetValues(const string &key, const vector<string> &values);

  // Accessors for convenience.

  string GetModuleNamePrefix() const;
  void SetModuleNamePrefix(const string &name);

  string GetPortNamePrefix() const;
  void SetPortNamePrefix(const string &name);

  string GetMappedName() const;
  void SetMappedName(const string &name);

  bool GetResetPolarity() const;
  void SetResetPolarity(bool p);
  bool HasResetPolarity() const;

  string GetResetName() const;
  void SetResetName(const string &name);

  int GetAddrWidth() const;
  void SetAddrWidth(int width);

  void SetExtInputPort(const string &input, int width);
  void GetExtInputPort(string *name, int *width);
  void SetExtOutputPort(const string &output, int width);
  void GetExtOutputPort(string *name, int *width);

  // DEFAULT-VALUE
  bool GetDefaultValue(int *value) const;
  int GetWidth();
  void SetWidth(int w);

  void SetEmbeddedModuleName(const string &mod, const string &fn);
  string GetEmbeddedModuleName() const;
  string GetEmbeddedModuleFileName() const;
  string GetEmbeddedModuleClk() const;
  string GetEmbeddedModuleReset() const;
  string GetEmbeddedModuleReq() const;
  string GetEmbeddedModuleAck() const;
  vector<string> GetEmbeddedModuleArgs() const;

  string GetChannelDataPort() const;
  string GetChannelEnPort() const;
  string GetChannelAckPort() const;

  string GetExtTaskName() const;

private:
  resource::ResourceParamValueSet *values_;
};

}  // namespace iroha

#endif  // _iroha_resource_params_h_
