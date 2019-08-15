#include "writer/verilog/ports.h"

#include "iroha/stl_util.h"

static const string kInput = "input";
static const string kOutput = "output";

namespace iroha {
namespace writer {
namespace verilog {

Ports::~Ports() {
  STLDeleteValues(&ports_);
}

Port *Ports::AddPort(const string &name, enum Port::PortType type,
		     int width) {
  return AddPrefixedPort("", name, type, width);
}

Port *Ports::AddPrefixedPort(const string &prefix, const string &name,
			     enum Port::PortType type, int width) {
  Port *p = new Port(prefix, name, type, width);
  ports_.push_back(p);
  if (type == Port::INPUT_CLK) {
    clk_ = name;
  }
  if (type == Port::INPUT_RESET) {
    reset_ = name;
  }
  return p;
}

const string &Ports::GetClk() const {
  return clk_;
}

const string &Ports::GetReset() const {
  return reset_;
}

void Ports::Output(enum OutputType type, ostream &os) const {
  bool is_first = true;
  for (Port *p : ports_) {
    OutputPort(p, type, is_first, os);
    is_first = false;
  }
}

void Ports::OutputPort(Port *p, enum OutputType type, bool is_first,
		       ostream &os) const {
  if (type == FIXED_VALUE_ASSIGN) {
    OutputFixedValueAssign(p, os);
    return;
  }
  if (type == PORT_CONNECTION_DATA) {
    if (!is_first) {
      os << ",";
    }
    os << DirectionPort(p->GetType()) << ":" << p->GetWidth() << ":"
       << p->GetPrefix() << ":" << p->GetSuffix();
    return;
  }
  if (type == PORT_MODULE_HEAD || type == PORT_MODULE_HEAD_DIRECTION) {
    if (is_first) {
      os << "\n";
    } else {
      os << ",\n";
    }
    Port::PortType port_type = p->GetType();
    os << "  " << DirectionPort(port_type);
    // omitted for PORT_MODULE_HEAD_DIRECTION
    if (type == PORT_MODULE_HEAD && port_type == Port::OUTPUT) {
      os << " reg";
    }
    if (p->GetWidth() > 0) {
      os << " [" << (p->GetWidth() - 1) << ":0]";
    }
    os << " " << p->GetName();
  }
  if (type == PORT_CONNECTION || type == PORT_CONNECTION_TEMPLATE) {
    if (!is_first) {
      os << ", ";
    }
    os << "." << p->GetName() << "(";
    if (type == PORT_CONNECTION) {
      os << p->GetName();
    }
    os << ")";
  }
}

void Ports::OutputFixedValueAssign(Port *p, ostream &os) const {
  int v = p->GetFixedValue();
  if (v < 0) {
    return;
  }
  os << "  assign " << p->GetName() << " = " << v << ";\n";
}

const string &Ports::DirectionPort(Port::PortType type) {
  if (type == Port::INPUT || type == Port::INPUT_CLK ||
      type == Port::INPUT_RESET) {
    return kInput;
  } else {
    return kOutput;
  }
}

Port::Port(const string &prefix, const string &name, enum PortType type, int width)
  : prefix_(prefix), name_(prefix + name), suffix_(name), type_(type), width_(width), fixed_value_(-1) {
}

Port::~Port() {
}

void Port::SetComment(const string &comment) {
  comment_ = comment;
}

const string &Port::GetPrefix() {
  return prefix_;
}

const string &Port::GetName() {
  return name_;
}

const string &Port::GetSuffix() {
  return suffix_;
}

enum Port::PortType Port::GetType() {
  return type_;
}

int Port::GetWidth() {
  return width_;
}

void Port::SetFixedValue(int fixed_value) {
  fixed_value_ = fixed_value;
}

int Port::GetFixedValue() const {
  return fixed_value_;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
