// -*- C++ -*-
#ifndef _writer_verilog_module_h_
#define _writer_verilog_module_h_

#include "iroha/common.h"

namespace iroha {

class IModule;
class ModuleTemplate;

namespace verilog {

class Embed;
class Ports;
class Table;

static const char kEmbeddedInstanceSection[] = "embedded";
static const char kSubModuleSection[] = "sub_modules";
static const char kStateDeclSection[] = "state_decl";
static const char kStateVarSection[] = "state_var";
static const char kRegisterSection[] = "register";
static const char kResourceSection[] = "resource";
static const char kInitialValueSection[] = "initial";

class Module {
public:
  Module(const IModule *i_mod, Embed *embed);
  ~Module();

  void Build();
  void Write(ostream &os);
  bool GetResetPolarity() const;
  const IModule *GetIModule() const;
  const Ports *GetPorts() const;
  void BuildChildModuleSection(vector<Module *> &mods);

private:

  const IModule *i_mod_;
  Embed *embed_;
  unique_ptr<ModuleTemplate> tmpl_;
  unique_ptr<Ports> ports_;
  vector<Table *> tables_;
  bool reset_polarity_;
};

}  // namespace verilog
}  // namespace iroha

#endif  // _writer_verilog_module_h_
