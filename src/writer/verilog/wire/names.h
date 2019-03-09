// -*- C++ -*-
#ifndef _writer_verilog_wire_names_h_
#define _writer_verilog_wire_names_h_

#include "writer/verilog/common.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

class Names {
public:
  static string AccessorName(const string &resource_name,
			     const IResource *res);
  static string AccessorSignalBase(const string &resource_name,
				   const IResource *res, const char *name);
  static string AccessorWire(const string &resource_name, const IResource *res,
			     const char *name);
  static string AccessorEdgeWire(const string &resource_name,
				 const IResource *res,
				 const char *name);

  static string ResourceSignalBase(const string &resource_name,
				   const char *name);
  static string ResourceWire(const string &resource_name, const char *name);

private:
  static string AccessorResourceName(const IResource *res);
};

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_wire_names_h_
