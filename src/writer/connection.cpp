#include "writer/connection.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"

#include <set>

namespace iroha {
namespace writer {

Connection::Connection(const IDesign *design) : design_(design) {
}

void Connection::Build() {
  for (auto *ch : design_->channels_) {
    ProcessChannel(ch);
  }
}

const ChannelInfo *Connection::GetConnectionInfo(const IModule *mod) const {
  auto it = channel_info_.find(mod);
  if (it == channel_info_.end()) {
    return nullptr;
  }
  const ChannelInfo &ci = it->second;
  return &ci;
}

void Connection::ProcessChannel(const IChannel *ch) {
  if (ch->GetReader() != nullptr &&
      ch->GetWriter() != nullptr) {
    // Internal.
    MakeInDesignChannelPath(ch);
  } else {
    // External R/W.
    MarkExtChannelPath(ch, ch->GetReader(), true);
    MarkExtChannelPath(ch, ch->GetWriter(), false);
  }
}

void Connection::MarkExtChannelPath(const IChannel *ch, const IResource *res,
				    bool parent_is_write) {
  if (!res) {
    return;
  }
  MakeSimpleChannelPath(ch, res->GetTable()->GetModule(), nullptr,
			parent_is_write);
}

void Connection::MakeInDesignChannelPath(const IChannel *ch) {
  const IModule *reader_mod = ch->GetReader()->GetTable()->GetModule();
  const IModule *writer_mod = ch->GetWriter()->GetTable()->GetModule();
  if (reader_mod == writer_mod) {
    return;
  }
  const IModule *common_root = GetCommonRoot(reader_mod, writer_mod);
  ChannelInfo &ci = channel_info_[common_root];
  ci.common_root_.push_back(ch);
  MakeSimpleChannelPath(ch, writer_mod, common_root, false);
  MakeSimpleChannelPath(ch, reader_mod, common_root, true);
}

void Connection::MakeSimpleChannelPath(const IChannel *ch,
				       const IModule *source,
				       const IModule *common_parent,
				       bool parent_is_write) {
  for (auto *mod = source; mod != common_parent;
       mod = mod->GetParentModule()) {
    AddChannelInfo(ch, mod, parent_is_write);
  }
}

void Connection::AddChannelInfo(const IChannel *ch, const IModule *mod,
				bool parent_is_write) {
  ChannelInfo &ci = channel_info_[mod];
  if (parent_is_write) {
    ci.downward_.push_back(ch);
  } else {
    ci.upward_.push_back(ch);
  }
  const IModule *parent = mod->GetParentModule();
  if (parent != nullptr) {
    ChannelInfo &pci = channel_info_[parent];
    if (parent_is_write) {
      pci.child_downward_[mod].push_back(ch);
    } else {
      pci.child_upward_[mod].push_back(ch);
    }
  }
}

const IModule *Connection::GetCommonRoot(const IModule *m1,
					 const IModule *m2) {
  set<const IModule *> parents;
  for (auto *m = m1; m != nullptr; m = m->GetParentModule()) {
    parents.insert(m);
  }
  for (auto *m = m2; m != nullptr; m = m->GetParentModule()) {
    if (parents.find(m) != parents.end()) {
      return m;
    }
  }
  return nullptr;
}

}  // namespace writer
}  // namespace iroha
