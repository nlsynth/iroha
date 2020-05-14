#include "writer/verilog/port.h"

namespace iroha {
namespace writer {
namespace verilog {

Port::Port(const string &prefix, const string &name, enum PortType type,
	   int width)
  : prefix_(prefix), name_(name), full_name_(prefix + name), type_(type),
    width_(width), fixed_value_(-1), is_axi_(false), is_axi_user_(false) {
}

Port::~Port() {
}

void Port::SetComment(const string &comment) {
  comment_ = comment;
}

const string &Port::GetPrefix() {
  return prefix_;
}

const string &Port::GetFullName() {
  return full_name_;
}

const string &Port::GetName() {
  return name_;
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

void Port::SetIsAxi(bool is_axi) {
  is_axi_ = is_axi;
  if (is_axi) {
    if (string(name_.c_str() + name_.size() -4) == "USER") {
      is_axi_user_ = true;
    }
  }
}

bool Port::GetIsAxi() {
  return is_axi_;
}

bool Port::GetIsAxiUser() {
  return is_axi_user_;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
