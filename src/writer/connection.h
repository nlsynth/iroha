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
  // In design channel.
  vector<const IChannel *> reader_from_up_;
  vector<const IChannel *> reader_from_down_;
  vector<const IChannel *> writer_to_up_;
  vector<const IChannel *> writer_to_down_;
  vector<const IChannel *> data_path_;
  // Channel to external.
  vector<const IChannel *> ext_reader_;
  vector<const IChannel *> ext_writer_;
  vector<const IChannel *> ext_reader_path_;
  vector<const IChannel *> ext_writer_path_;
};

class Connection {
public:
  Connection(const IDesign *design);
  void Build();
  const ChannelInfo *GetConnectionInfo(const IModule *mod) const;

private:
  void ProcessChannel(const IChannel *ch);
  void MarkExtChannelPath(const IChannel *ch, const IResource *res,
			  bool is_read);
  void MakeInDesignChannelPath(const IChannel *ch);
  const IModule *GetCommonRoot(const IModule *m1, const IModule *m2);
  void MakeSimpleChannelPath(const IChannel *ch, const IModule *parent,
			     const IModule *child, bool parent_is_write);

  const IDesign *design_;
  map<const IModule *, ChannelInfo> channel_info_;
};

}  // namespace writer
}  // namespace iroha

#endif  // _writer_connection_h_
