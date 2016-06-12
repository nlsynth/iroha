// -*- C++ -*-
#ifndef _writer_verilog_channel_h_
#define _writer_verilog_channel_h_

#include "writer/verilog/common.h"
#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class Channel : public Resource {
public:
  Channel(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static void BuildChannelPorts(const ChannelInfo &ci, Ports *ports);
  static void BuildChannelWire(const ChannelInfo &ci, const IModule *child_mod,
			       ostream &os);

  static string DataPort(const IChannel &ic);
  static string AckPort(const IChannel &ic);
  static string EnPort(const IChannel &ic);

private:
  static string PortName(const IChannel &ic, const string &type);
  static void BuildChildModuleChannelWire(const IChannel &ch, ostream &is);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_channel_h_
