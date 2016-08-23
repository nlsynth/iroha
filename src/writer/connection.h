//  -*- C++ -*-
#ifndef _writer_connection_h_
#define _writer_connection_h_

#include "iroha/common.h"

#include <map>
#include <set>

namespace iroha {
namespace writer {

// Per module channel path information.
class ChannelInfo {
public:
  // This module has data output ports.
  vector<const IChannel *> upward_;
  // This module has data input ports.
  vector<const IChannel *> downward_;
  // Highest module in the path.
  vector<const IChannel *> common_root_;

  // child -> channels.
  map<const IModule *, vector<const IChannel *> > child_upward_;
  map<const IModule *, vector<const IChannel *> > child_downward_;
};

// Per module task call path information.
class TaskCallInfo {
public:
  // Callers for this or sub modules.
  vector<IResource *> tasks_;
};

// Per module foreign register access connections.
class RegConnectionInfo {
public:
  set<IRegister *> has_upward_port;
  set<IRegister *> has_downward_port;
  set<IRegister *> has_wire;
  set<IRegister *> is_source;
};

class Connection {
public:
  Connection(const IDesign *design);
  void Build();
  const ChannelInfo *GetConnectionInfo(const IModule *mod) const;
  const TaskCallInfo *GetTaskCallInfo(const IModule *mod) const;
  const RegConnectionInfo *GetRegConnectionInfo(const IModule *mod) const;

private:
  // Channel.
  void ProcessChannel(const IChannel *ch);
  void MarkExtChannelPath(const IChannel *ch, const IResource *res,
			  bool parent_is_write);
  void MakeInDesignChannelPath(const IChannel *ch);
  const IModule *GetCommonRoot(const IModule *m1, const IModule *m2);
  void MakeSimpleChannelPath(const IChannel *ch,
			     const IModule *source,
			     const IModule *common_parent,
			     bool parent_is_write);
  void AddChannelInfo(const IChannel *ch, const IModule *mod,
		      bool parent_is_write);

  // Sub module task call.
  void ProcessSubModuleTaskCall(IResource *caller);
  void ProcessForeignReg(IResource *freg);

  const IDesign *design_;
  map<const IModule *, ChannelInfo> channel_info_;
  map<const IModule *, TaskCallInfo> task_call_info_;
  map<const IModule *, RegConnectionInfo> reg_connection_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_connection_h_
