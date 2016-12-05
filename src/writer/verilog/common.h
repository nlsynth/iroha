// -*- C++ -*-
#ifndef _writer_verilog_common_h_
#define _writer_verilog_common_h_

#include "iroha/common.h"

namespace iroha {
namespace writer {

class ChannelInfo;
class Connection;
class ModuleTemplate;
class Names;
class TaskCallInfo;
class RegConnectionInfo;
class ResourceConnectionInfo;

namespace verilog {

class DataFlowState;
class DataFlowTable;
class EmbeddedModules;
class InternalSRAM;
class Module;
class Resource;
class State;
class Table;
class Task;
class Ports;
class VerilogWriter;

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_common_h_
