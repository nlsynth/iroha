// -*- C++ -*-
#ifndef _writer_verilog_axi_channel_generator_h_
#define _writer_verilog_axi_channel_generator_h_

#include "writer/verilog/axi/axi_port.h"  // for PortConfig

namespace iroha {
namespace writer {
namespace verilog {
namespace axi {

class ChannelGenerator {
public:
  enum OutputType {
    CONTROLLER_PORTS_AND_REG_INITIALS,
    PORTS_TO_EXT_AND_CONNECTIONS,
  };
  ChannelGenerator(const PortConfig &cfg, enum OutputType type,
		   bool is_master,
		   Module *module, Ports *ports, string *s);

  void GenerateChannel(bool r, bool w);

private:
  void GenReadChannel();
  void GenWriteChannel();
  // module==nullptr: Can get register initializer.
  // module!=nullptr: Child module wirings.
  void AddPort(const string &name, int width,
	       bool dir_s2m,
	       // valid if non negative.
	       int fixed_value);
  void MayAddInitialRegValue(const string &name, int width, int fixed_value);

  const PortConfig &cfg_;
  enum OutputType type_;
  bool is_master_;
  Module *module_;
  Ports *ports_;
  string *s_;
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_channel_generator_h_
