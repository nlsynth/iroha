// -*- C++ -*-
#ifndef _writer_verilog_port_set_h_
#define _writer_verilog_port_set_h_

#include "writer/verilog/common.h"
#include "writer/verilog/port.h"

namespace iroha {
namespace writer {
namespace verilog {

class PortSet {
public:
  enum OutputType {
    // SystemVerilog style module head.
    PORT_MODULE_HEAD,
    // SystemVerilog style module head with only the direction.
    PORT_MODULE_HEAD_DIRECTION,
    // Pin connection from internal module ".name(name)"
    PORT_CONNECTION,
    // Pin connection template for outer module ".name()"
    PORT_CONNECTION_TEMPLATE,
    // Pin connection info in machine readable format.
    PORT_CONNECTION_DATA,
    // Assigns a fixed value if specified "assign name = v;".
    FIXED_VALUE_ASSIGN,
  };

  ~PortSet();

  Port *AddPort(const string &name, enum Port::PortType type, int width);
  // e.g. "m00_" + "AWADDR".
  Port *AddPrefixedPort(const string &prefix, const string &name,
			enum Port::PortType type, int width);
  void Output(enum OutputType type, ostream &os) const;
  const string &GetClk() const;
  const string &GetReset() const;

private:
  void OutputPort(Port *p, enum OutputType type, bool is_first,
		  ostream &os) const;
  void OutputFixedValueAssign(Port *port, ostream &os) const;
  static const string &DirectionPort(Port::PortType type);

  vector<Port *> ports_;
  string clk_;
  string reset_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_port_set_h_
