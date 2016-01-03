#include "writer/connection.h"

#include "iroha/i_design.h"
#include "iroha/logging.h"

#include <set>

namespace iroha {

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
				    bool is_read) {
  if (!res) {
    return;
  }
  const IModule *mod = res->GetTable()->GetModule();
  ChannelInfo &ci = channel_info_[mod];
  if (is_read) {
    ci.ext_reader_.push_back(ch);
  } else {
    ci.ext_writer_.push_back(ch);
  }
  for (const IModule *parent = mod->GetParentModule();
       parent != nullptr; parent = parent->GetParentModule()) {
    ChannelInfo &pci = channel_info_[parent];
    if (is_read) {
      pci.ext_reader_path_.push_back(ch);
    } else {
      pci.ext_writer_path_.push_back(ch);
    }
  }
}

void Connection::MakeInDesignChannelPath(const IChannel *ch) {
  const IModule *reader_mod = ch->GetReader()->GetTable()->GetModule();
  const IModule *writer_mod = ch->GetWriter()->GetTable()->GetModule();
  CHECK(reader_mod != writer_mod);
  const IModule *common_root = GetCommonRoot(reader_mod, writer_mod);
  if (common_root == reader_mod) {
    MakeSimpleChannelPath(ch, common_root, writer_mod, false);
  }
  if (common_root == writer_mod) {
    MakeSimpleChannelPath(ch, common_root, reader_mod, true);
  }
}

void Connection::MakeSimpleChannelPath(const IChannel *ch,
				       const IModule *parent,
				       const IModule *child,
				       bool parent_is_write) {
  ChannelInfo &cci = channel_info_[child];
  ChannelInfo &pci = channel_info_[parent];
  if (parent_is_write) {
    pci.writer_to_down_.push_back(ch);
    cci.reader_from_up_.push_back(ch);
  } else {
    pci.writer_to_up_.push_back(ch);
    cci.reader_from_down_.push_back(ch);
  }
  for (auto *mod = child->GetParentModule(); mod != parent;
       parent = parent->GetParentModule()) {
    ChannelInfo &ci = channel_info_[child];
    ci.data_path_.push_back(ch);
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

}  // namespace iroha
