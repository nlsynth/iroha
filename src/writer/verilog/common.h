// -*- C++ -*-
#ifndef _writer_verilog_common_h_
#define _writer_verilog_common_h_

#include "iroha/common.h"

namespace iroha {
namespace writer {

class ChannelInfo;
class Connection;
class ModuleTemplate;
class TaskCallInfo;
class RegConnectionInfo;
class SharedRegConnectionInfo;

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

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_common_h_
