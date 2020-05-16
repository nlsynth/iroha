#include "writer/verilog/port.h"

namespace iroha {
namespace writer {
namespace verilog {

Port::Port(const string &prefix, const string &name, enum PortType type,
	   int width)
  : prefix_(prefix), name_(name), full_name_(prefix + name), type_(type),
    width_(width), axi_addr_width_(0), fixed_value_(-1), is_axi_(false) {
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
}

bool Port::GetIsAxi() {
  return is_axi_;
}

void Port::SetAxiAddrWidth(int axi_addr_width) {
  axi_addr_width_ = axi_addr_width;
}

int Port::GetAxiAddrWidth() {
  return axi_addr_width_;
}

}  // namespace verilog
}  // namespace writer
}  // namespace iroha
