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
  const RegConnectionInfo *GetRegConnectionInfo(const IModule *mod) const;
  const vector<IResource *> *GetSharedRegWriters(const IResource *res) const;
  const vector<IResource *> *GetSharedRegReaders(const IResource *res) const;
  const vector<IResource *> *GetSharedMemoryAccessors(const IResource *res) const;
  const vector<IResource *> *GetSharedMemoryPorts(const IResource *res) const;
  const vector<IResource *> *GetTaskCallers(const IResource *res) const;
  static const IModule *GetCommonRoot(const IModule *m1, const IModule *m2);

private:
  void ProcessChannel(const IChannel *ch);
  void MarkExtChannelPath(const IChannel *ch, const IResource *res,
			  bool parent_is_write);
  void MakeInDesignChannelPath(const IChannel *ch);
  void MakeSimpleChannelPath(const IChannel *ch,
			     const IModule *source,
			     const IModule *common_parent,
			     bool parent_is_write);
  void AddChannelInfo(const IChannel *ch, const IModule *mod,
		      bool parent_is_write);
  void ProcessForeignReg(IResource *freg);
  const vector<IResource *> *GetResourceVector(const map<const IResource *, vector<IResource *>> &m, const IResource *res) const;
  void ProcessTable(ITable *tab);

  const IDesign *design_;
  map<const IModule *, ChannelInfo> channel_info_;
  map<const IModule *, RegConnectionInfo> reg_connection_;
  map<const IResource *, vector<IResource *>> shared_reg_readers_;
  map<const IResource *, vector<IResource *>> shared_reg_writers_;
  map<const IResource *, vector<IResource *>> shared_memory_accessors_;
  map<const IResource *, vector<IResource *>> shared_memory_ports_;
  map<const IResource *, vector<IResource *>> task_callers_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_connection_h_
