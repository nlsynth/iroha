#include "writer/verilog/port.h"

namespace iroha {
namespace writer {
namespace verilog {

Port::Port(const string &prefix, const string &name, enum PortType type,
	   int width)
  : prefix_(prefix), name_(prefix + name), suffix_(name), type_(type),
    width_(width), fixed_value_(-1) {
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
