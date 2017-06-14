#include "writer/connection.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/logging.h"
#include "iroha/stl_util.h"

#include <set>

namespace iroha {
namespace writer {

Connection::Connection(const IDesign *design) : design_(design) {
}

Connection::~Connection() {
  STLDeleteSecondElements(&channel_info_);
  STLDeleteSecondElements(&reg_connection_);
}

void Connection::Build() {
  for (auto *ch : design_->channels_) {
    ProcessChannel(ch);
  }
  for (auto *mod : design_->modules_) {
    for (auto *tab : mod->tables_) {
      ProcessTable(tab);
    }
  }
}

void Connection::ProcessTable(ITable *tab) {
  // foreign-reg
  vector<IResource*> foreign_regs;
  DesignUtil::FindResourceByClassName(tab, resource::kForeignReg,
				      &foreign_regs);
  for (IResource *freg : foreign_regs) {
    ProcessForeignReg(freg);
  }
  // task
  vector<IResource *> task_callers;
  DesignUtil::FindResourceByClassName(tab, resource::kTaskCall,
				      &task_callers);
  for (IResource *reg : task_callers) {
    vector<IResource *> res;
    DesignUtil::FindResourceByClassName(reg->GetCalleeTable(),
					resource::kTask,
					&res);
    CHECK(res.size() == 1);
    task_callers_[res[0]].push_back(reg);
  }
  // shared-reg
  vector<IResource*> reg_writers;
  DesignUtil::FindResourceByClassName(tab, resource::kSharedRegWriter,
				      &reg_writers);
  for (IResource *reg : reg_writers) {
    shared_reg_writers_[reg->GetParentResource()].push_back(reg);
  }
  vector<IResource*> reg_readers;
  DesignUtil::FindResourceByClassName(tab, resource::kSharedRegReader,
				      &reg_readers);
  for (IResource *reg : reg_readers) {
    shared_reg_readers_[reg->GetParentResource()].push_back(reg);
  }
  vector<IResource*> reg_children;
  DesignUtil::FindResourceByClassName(tab, resource::kDataFlowIn,
				      &reg_children);
  for (IResource *reg : reg_children) {
    IResource *p = reg->GetParentResource();
    if (p != nullptr) {
      shared_reg_children_[p].push_back(reg);
    }
  }
  // shared-memory
  vector<IResource *> memory_readers;
  DesignUtil::FindResourceByClassName(tab, resource::kSharedMemoryReader,
				      &memory_readers);
  for (IResource *reg : memory_readers) {
    shared_memory_accessors_[reg->GetParentResource()].push_back(reg);
  }
  vector<IResource *> memory_writers;
  DesignUtil::FindResourceByClassName(tab, resource::kSharedMemoryWriter,
				      &memory_writers);
  for (IResource *reg : memory_writers) {
    shared_memory_accessors_[reg->GetParentResource()].push_back(reg);
  }
  vector<IResource *> master_ports;
  DesignUtil::FindResourceByClassName(tab, resource::kAxiMasterPort,
				      &master_ports);
  for (IResource *reg : master_ports) {
    shared_memory_ports_[reg->GetParentResource()].push_back(reg);
  }
  vector<IResource *> slave_ports;
  DesignUtil::FindResourceByClassName(tab, resource::kAxiSlavePort,
				      &slave_ports);
  for (IResource *reg : slave_ports) {
    shared_memory_ports_[reg->GetParentResource()].push_back(reg);
  }
}

const ChannelInfo *Connection::GetChannelInfo(const IModule *mod) const {
  auto it = channel_info_.find(mod);
  if (it == channel_info_.end()) {
    return nullptr;
  }
  return it->second;
}

const RegConnectionInfo *Connection::GetRegConnectionInfo(const IModule *mod) const {
  auto it = reg_connection_.find(mod);
  if (it == reg_connection_.end()) {
    return nullptr;
  }
  return it->second;
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
  ChannelInfo *ci = FindChannelInfo(common_root);
  ci->common_root_.push_back(ch);
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
  ChannelInfo *ci = FindChannelInfo(mod);
  if (parent_is_write) {
    ci->downward_.push_back(ch);
  } else {
    ci->upward_.push_back(ch);
  }
  const IModule *parent = mod->GetParentModule();
  if (parent != nullptr) {
    ChannelInfo *pci = FindChannelInfo(parent);
    if (parent_is_write) {
      pci->child_downward_[mod].push_back(ch);
    } else {
      pci->child_upward_[mod].push_back(ch);
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

const vector<IResource *> *Connection::GetSharedRegReaders(const IResource *res) const {
  return GetResourceVector(shared_reg_readers_, res);
}

const vector<IResource *> *Connection::GetSharedRegWriters(const IResource *res) const {
  return GetResourceVector(shared_reg_writers_, res);
}

const vector<IResource *> *Connection::GetSharedRegChildren(const IResource *res) const {
  return GetResourceVector(shared_reg_children_, res);
}
  
const vector<IResource *> *Connection::GetSharedMemoryAccessors(const IResource *res) const {
  return GetResourceVector(shared_memory_accessors_, res);
}

const vector<IResource *> *Connection::GetSharedMemoryPorts(const IResource *res) const {
  return GetResourceVector(shared_memory_ports_, res);
}

const vector<IResource *> *Connection::GetTaskCallers(const IResource *res) const {
  return GetResourceVector(task_callers_, res);
}

const vector<IResource *> *Connection::GetResourceVector(const map<const IResource *, vector<IResource *>> &m, const IResource *res) const {
  auto it = m.find(res);
  if (it != m.end() && it->second.size() > 0) {
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
  RegConnectionInfo *sc = FindRegConnectionInfo(reg_module);
  sc->is_source.insert(reg);
  const IModule *common_root = GetCommonRoot(reg_module, reader_module);
  if (common_root == reg_module) {
    // reg is uppper and the accessor is below.
    for (IModule *mod = reader_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo *rc = FindRegConnectionInfo(mod);
      rc->has_downward_port.insert(reg);
    }
  } else if (common_root == reader_module) {
    for (IModule *mod = reg_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo *rc = FindRegConnectionInfo(mod);
      rc->has_upward_port.insert(reg);
    }
    RegConnectionInfo *rc = FindRegConnectionInfo(common_root);
    rc->has_wire.insert(reg);
  } else {
    RegConnectionInfo *cc = FindRegConnectionInfo(common_root);
    cc->has_wire.insert(reg);
    for (IModule *mod = reader_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo *rc = FindRegConnectionInfo(mod);
      rc->has_upward_port.insert(reg);
    }
    for (IModule *mod = reg_module; mod != common_root;
	 mod = mod->GetParentModule()) {
      RegConnectionInfo *rc = FindRegConnectionInfo(mod);
      rc->has_downward_port.insert(reg);
    }
  }
}

ChannelInfo *Connection::FindChannelInfo(const IModule *mod) {
  auto it = channel_info_.find(mod);
  if (it == channel_info_.end()) {
    ChannelInfo *ci = new ChannelInfo();
    channel_info_[mod] = ci;
    return ci;
  }
  return it->second;
}

RegConnectionInfo *Connection::FindRegConnectionInfo(const IModule *mod) {
  auto it = reg_connection_.find(mod);
  if (it == reg_connection_.end()) {
    RegConnectionInfo *ri = new RegConnectionInfo();
    reg_connection_[mod] = ri;
    return ri;
  }
  return it->second;
}

}  // namespace writer
}  // namespace iroha
