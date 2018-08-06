// -*- C++ -*-
#ifndef _writer_verilog_ports_h_
#define _writer_verilog_ports_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {

class Port {
public:
  enum PortType {
    INPUT,
    OUTPUT,
    // Clock and reset.
    INPUT_CLK,
    INPUT_RESET,
    // Comes from inner module.
    OUTPUT_WIRE,
  };

  Port(const string &prefix, const string &name, enum PortType type, int width);
  ~Port();

  void SetComment(const string &comment);
  const string &GetPrefix();
  const string &GetName();
  const string &GetSuffix();
  enum PortType GetType();
  int GetWidth();
  void SetFixedValue(int default_value);
  int GetFixedValue() const;

private:
  string prefix_;
  string name_;
  string suffix_;
  enum PortType type_;
  int width_;
  string comment_;
  // valid if non negative.
  int fixed_value_;
};

class Ports {
public:
  enum OutputType {
    // Only port names for module head.
    PORT_NAME,
    // Input and output. Also generate reg for output.
    PORT_TYPE,
    // Only input/output. Used to generate top level wrapper.
    PORT_DIRECTION,
    // Pin connection from internal module ".name(name)"
    PORT_CONNECTION,
    // Pin connection template for outer module ".name()"
    PORT_CONNECTION_TEMPLATE,
    // Pin connection info in machine readable format.
    PORT_CONNECTION_DATA,
    // Assigns a fixed value if specified "assign name = v;".
    FIXED_VALUE_ASSIGN,
  };

  ~Ports();

  Port *AddPort(const string &name, enum Port::PortType type, int width);
  Port *AddPrefixedPort(const string &prefix, const string &name,
			enum Port::PortType type, int width);
  void Output(enum OutputType type, ostream &os) const;
  const string &GetClk() const;
  const string &GetReset() const;

private:
  void OutputPort(Port *p, enum OutputType type, bool is_first,
		  bool reg_phase, ostream &os) const;
  void OutputFixedValueAssign(Port *port, ostream &os) const;
  static const string &DirectionPort(Port::PortType type);

  vector<Port *> ports_;
  string clk_;
  string reset_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ports_h_

