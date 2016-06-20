//  -*- C++ -*-
#ifndef _writer_connection_h_
#define _writer_connection_h_

#include "iroha/common.h"

#include <map>

namespace iroha {
namespace writer {

// Per module information.
class ChannelInfo {
public:
  // This module has data output ports.
  vector<const IChannel *> upward_;
  // This module has data input ports.
  vector<const IChannel *> downward_;
  // Highest module in the path.
  vector<const IChannel *> common_root_;

  // child -> channels.
  map<const IModule *, vector<const IChannel *>> child_upward_;
  map<const IModule *, vector<const IChannel *>> child_downward_;
};

class Connection {
public:
  Connection(const IDesign *design);
  void Build();
  const ChannelInfo *GetConnectionInfo(const IModule *mod) const;

private:
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

  const IDesign *design_;
  map<const IModule *, ChannelInfo> channel_info_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_connection_h_
