// -*- C++ -*-
#ifndef _writer_verilog_module_h_
#define _writer_verilog_module_h_

#include "iroha/common.h"

namespace iroha {
namespace writer {

class ChannelInfo;
class Connection;
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
static const char kInitialValueSection[] = "initial";  // With table id.
static const char kInsnWireDeclSection[] = "insn_wire_decl";
static const char kInsnWireValueSection[] = "insn_wire_value";

class Module {
public:
  Module(const IModule *i_mod, const Connection &conn, Embed *embed);
  ~Module();

  void Build();
  void Write(ostream &os);
  bool GetResetPolarity() const;
  const IModule *GetIModule() const;
  const Ports *GetPorts() const;
  void BuildChildModuleSection(vector<Module *> &mods);

private:
  void BuildChannelConnections(const ChannelInfo &ci);
  void BuildChildModuleChannelWire(const IChannel &ch, ostream &is);

  const IModule *i_mod_;
  const Connection &conn_;
  Embed *embed_;
  unique_ptr<ModuleTemplate> tmpl_;
  unique_ptr<Ports> ports_;
  vector<Table *> tables_;
  bool reset_polarity_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_module_h_
