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
  // module==nullptr: Can get register initializer.
  // module!=nullptr: Child module wirings.
  static void AddPort(const PortConfig &cfg, const string &name, int width,
		      bool dir_s2m,
		      bool is_master,
		      // valid if non negative.
		      int fixed_value,
		      Module *module, Ports *ports, string *s);
  static void GenReadChannel(const PortConfig &cfg,
			     bool is_master, Module *module, Ports *ports,
			     string *s);
  static void GenWriteChannel(const PortConfig &cfg,
			      bool is_master, Module *module, Ports *ports,
			      string *s);
};

}  // namespace axi
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_axi_channel_generator_h_
