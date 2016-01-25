// -*- C++ -*-
#ifndef _writer_verilog_module_h_
#define _writer_verilog_module_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

// For module.
static const char kStateDeclSection[] = "state_decl";
static const char kStateVarSection[] = "state_var";
static const char kRegisterSection[] = "register";
static const char kResourceSection[] = "resource";
static const char kInsnWireDeclSection[] = "insn_wire_decl";
static const char kInsnWireValueSection[] = "insn_wire_value";
static const char kEmbeddedInstanceSection[] = "embedded";
static const char kSubModuleSection[] = "sub_modules";
// For each table (with table id).
static const char kInitialValueSection[] = "initial";
static const char kStateOutput[] = "state_output";

class Module {
public:
  Module(const IModule *i_mod, const Connection &conn, Embed *embed);
  ~Module();

  void Build();
  void Write(ostream &os);
  bool GetResetPolarity() const;
  const IModule *GetIModule() const;
  const Ports *GetPorts() const;
  // Called from VerilogWriter.
  const vector<InternalSRAM *> &GetInternalSRAMs() const;
  void BuildChildModuleSection(vector<Module *> &mods);
  // Called from Table.
  InternalSRAM *RequestInternalSRAM(const IResource &res);

private:
  void BuildChannelConnections(const ChannelInfo &ci);
  void BuildChildModuleChannelWire(const IChannel &ch, ostream &is);

  const IModule *i_mod_;
  const Connection &conn_;
  Embed *embed_;
  unique_ptr<ModuleTemplate> tmpl_;
  unique_ptr<Ports> ports_;
  vector<Table *> tables_;
  vector<InternalSRAM *> srams_;
  bool reset_polarity_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_module_h_
