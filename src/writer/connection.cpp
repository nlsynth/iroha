#include "writer/connection.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
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
  for (auto *mod : design_->modules_) {
    for (auto *tab : mod->tables_) {
      vector<IResource*> task_calls;
      DesignUtil::FindResourceByClassName(tab, resource::kSubModuleTaskCall,
					  &task_calls);
      for (IResource *call : task_calls) {
	ProcessSubModuleTaskCall(call);
      }
      vector<IResource*> foreign_regs;
      DesignUtil::FindResourceByClassName(tab, resource::kForeignReg,
					  &foreign_regs);
      for (IResource *freg : foreign_regs) {
	ProcessForeignReg(freg);
      }
      vector<IResource*> reg_readers;
      DesignUtil::FindResourceByClassName(tab, resource::kSharedRegReader,
					  &reg_readers);
      for (IResource *reg : reg_readers) {
	ProcessResourceConnection(reg->GetSharedRegister(),
				  reg->GetTable()->GetModule(),
				  shared_reg_reader_);
      }
      vector<IResource*> reg_writers;
      DesignUtil::FindResourceByClassName(tab, resource::kSharedRegWriter,
					  &reg_writers);
      for (IResource *reg : reg_writers) {
	ProcessResourceConnection(reg,
				  reg->GetSharedRegister()->GetTable()->GetModule(), shared_reg_writer_);
	shared_reg_writers_[reg->GetSharedRegister()].push_back(reg);
      }
    }
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

const TaskCallInfo *Connection::GetTaskCallInfo(const IModule *mod) const {
  auto it = task_call_info_.find(mod);
  if (it == task_call_info_.end()) {
    return nullptr;
  }
  const TaskCallInfo &ti = it->second;
  return &ti;
}

const RegConnectionInfo *Connection::GetRegConnectionInfo(const IModule *mod) const {
  auto it = reg_connection_.find(mod);
  if (it == reg_connection_.end()) {
    return nullptr;
  }
  const RegConnectionInfo &ri = it->second;
  return &ri;
}

const ResourceConnectionInfo *
Connection::GetSharedRegReaderConnectionInfo(const IModule *mod) const {
  auto it = shared_reg_reader_.find(mod);
  if (it == shared_reg_reader_.end()) {
    return nullptr;
  }
  const ResourceConnectionInfo &pi = it->second;
  return &pi;
}

const ResourceConnectionInfo *
Connection::GetSharedRegWriterConnectionInfo(const IModule *mod) const {
  auto it = shared_reg_writer_.find(mod);
  if (it == shared_reg_writer_.end()) {
    return nullptr;
  }
  const ResourceConnectionInfo &pi = it->second;
  return &pi;
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

void Connection::ProcessSubModuleTaskCall(IResource *caller) {
  IModule *caller_mod = caller->GetTable()->GetModule();
  IModule *callee_mod = caller->GetCalleeTable()->GetModule();
  for (IModule *mod = callee_mod; mod != caller_mod;
       mod = mod->GetParentModule()) {
    TaskCallInfo &ti = task_call_info_[mod];
    ti.tasks_.push_back(caller);
  }
}

const vector<IResource *> *Connection::GetSharedRegWriters(const IResource *res) const {
  auto it = shared_reg_writers_.find(res);
  if (it != shared_reg_writers_.end() && it->second.size() > 0) {
    return &(it->second);
  }
  return nullptr;
}

void Connection::ProcessForeignReg(IResource *freg) {
  IRegister *reg = freg->GetForeignRegister();
  IModule *reg_module = reg->GetTable()->GetModule();
  IModule *reader_module = freg->GetTable()->GetModule();
  if (reg_module == reader_module) {
    return;
  }
  RegConnectionInfo &sc = reg_connection_[reg_module];
  sc.is_source.insert(reg);
  const IModule *common_root = GetCommonRoot(reg_module, reader_module);
  if (common_root == reg_module) {
    // reg is uppper and the accessor is below.
    for (IModule *mod = reader_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo &rc = reg_connection_[mod];
      rc.has_downward_port.insert(reg);
    }
  } else if (common_root == reader_module) {
    for (IModule *mod = reg_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo &rc = reg_connection_[mod];
      rc.has_upward_port.insert(reg);
    }
    RegConnectionInfo &rc = reg_connection_[common_root];
    rc.has_wire.insert(reg);
  } else {
    RegConnectionInfo &cc = reg_connection_[common_root];
    cc.has_wire.insert(reg);
    for (IModule *mod = reader_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo &rc = reg_connection_[mod];
      rc.has_upward_port.insert(reg);
    }
    for (IModule *mod = reg_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo &rc = reg_connection_[mod];
      rc.has_downward_port.insert(reg);
    }
  }
}

void Connection::ProcessResourceConnection(IResource *source, IModule *sink_module,
					   map<const IModule *, ResourceConnectionInfo> &conn_map) {
  IModule *source_module = source->GetTable()->GetModule();
  if (source_module == sink_module) {
    return;
  }
  const IModule *common_root = GetCommonRoot(source_module, sink_module);
  if (common_root == source_module) {
    // source is upper
    for (IModule *mod = sink_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      ResourceConnectionInfo &pc = conn_map[mod];
      pc.has_downward_port.insert(source);
    }
  } else if (common_root == sink_module) {
    for (IModule *mod = source_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      ResourceConnectionInfo &pc = conn_map[mod];
      pc.has_upward_port.insert(source);
    }
    ResourceConnectionInfo &pc = conn_map[common_root];
    pc.has_wire.insert(source);
  } else {
    ResourceConnectionInfo &pc = conn_map[common_root];
    pc.has_wire.insert(source);
    for (IModule *mod = source_module; mod != common_root;
	  mod = mod->GetParentModule()) {
      ResourceConnectionInfo &pc = conn_map[mod];
      pc.has_upward_port.insert(source);
    }
    for (IModule *mod = sink_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      ResourceConnectionInfo &pc = conn_map[mod];
      pc.has_downward_port.insert(source);
    }
  }
}

}  // namespace writer
}  // namespace iroha
