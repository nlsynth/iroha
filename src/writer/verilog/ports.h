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

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_ports_h_

