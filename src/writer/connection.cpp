#include "writer/connection.h"

#include "design/design_util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"
#include "iroha/logging.h"
#include "iroha/stl_util.h"

#include <set>

namespace iroha {
namespace writer {

Connection::Connection(const IDesign *design) : design_(design) {
}

Connection::~Connection() {
  STLDeleteSecondElements(&reg_connection_);
  STLDeleteSecondElements(&accessors_);
}

void Connection::Build() {
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
  for (IResource *caller : task_callers) {
    vector<IResource *> res;
    DesignUtil::FindResourceByClassName(caller->GetCalleeTable(),
					resource::kTask,
					&res);
    CHECK(res.size() == 1);
    auto *ai = FindAccessorInfo(res[0]);
    ai->task_callers_.push_back(caller);
  }
  // shared-reg
  ProcessSharedRegAccessors(tab);
  // shared-memory
  ProcessSharedMemoryAccessors(tab);
  // fifo
  ProcessFifoAccessors(tab);
}

void Connection::ProcessSharedRegAccessors(ITable *tab) {
  for (IResource *res : tab->resources_) {
    IResource *p = res->GetParentResource();
    if (p == nullptr ||
	!resource::IsSharedReg(*(p->GetClass()))) {
      continue;
    }
    auto *ai = FindAccessorInfo(p);
    auto *rc = res->GetClass();
    if (resource::IsSharedRegWriter(*rc)) {
      ai->shared_reg_writers_.push_back(res);
    }
    if (resource::IsSharedRegReader(*rc)) {
      ai->shared_reg_readers_.push_back(res);
    }
    if (resource::IsDataFlowIn(*rc)) {
      ai->shared_reg_children_.push_back(res);
    }
  }
}

void Connection::ProcessSharedMemoryAccessors(ITable *tab) {
  for (IResource *res : tab->resources_) {
    auto *ai = FindAccessorInfo(res->GetParentResource());
    auto *rc = res->GetClass();
    auto *params = res->GetParams();
    if (resource::IsSharedMemoryReader(*rc) ||
	resource::IsSharedMemoryWriter(*rc)) {
      ai->shared_memory_accessors_.push_back(res);
    }
    if (resource::IsAxiMasterPort(*rc) ||
	resource::IsAxiSlavePort(*rc)) {
      if (params->GetSramPortIndex() == "0") {
	ai->shared_memory_accessors_.push_back(res);
      } else {
	// Exclusive access only by a DMAC.
	ai->shared_memory_port1_accessors_.push_back(res);
      }
    }
  }
}

void Connection::ProcessFifoAccessors(ITable *tab) {
  for (IResource *res : tab->resources_) {
    IResource *p = res->GetParentResource();
    if (p == nullptr ||
	!resource::IsFifo(*(p->GetClass()))) {
      continue;
    }
    auto *ai = FindAccessorInfo(p);
    auto *rc = res->GetClass();
    auto *params = res->GetParams();
    if (resource::IsFifoReader(*rc)) {
      ai->fifo_readers_.push_back(res);
    }
    if (resource::IsFifoWriter(*rc)) {
      ai->fifo_writers_.push_back(res);
    }
    if (resource::IsDataFlowIn(*rc)) {
      ai->fifo_readers_.push_back(res);
    }
  }
}

const RegConnectionInfo *Connection::GetRegConnectionInfo(const IModule *mod) const {
  auto it = reg_connection_.find(mod);
  if (it == reg_connection_.end()) {
    return nullptr;
  }
  return it->second;
}

const AccessorInfo *Connection::GetAccessorInfo(const IResource *res) const {
  auto it = accessors_.find(res);
  if (it == accessors_.end()) {
    return &empty_accessors_;
  }
  return it->second;
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

const vector<IResource *> &Connection::GetSharedRegReaders(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->shared_reg_readers_;
}

const vector<IResource *> &Connection::GetSharedRegWriters(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->shared_reg_writers_;
}

const vector<IResource *> &Connection::GetSharedRegChildren(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->shared_reg_children_;
}
  
const vector<IResource *> &Connection::GetSharedMemoryAccessors(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->shared_memory_accessors_;
}

const vector<IResource *> &Connection::GetSharedMemoryPort1Accessors(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->shared_memory_port1_accessors_;
}

const vector<IResource *> &Connection::GetFifoWriters(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->fifo_writers_;
}

const vector<IResource *> &Connection::GetFifoReaders(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->fifo_readers_;
}

const vector<IResource *> &Connection::GetTaskCallers(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->task_callers_;
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

RegConnectionInfo *Connection::FindRegConnectionInfo(const IModule *mod) {
  auto it = reg_connection_.find(mod);
  if (it == reg_connection_.end()) {
    RegConnectionInfo *ri = new RegConnectionInfo();
    reg_connection_[mod] = ri;
    return ri;
  }
  return it->second;
}

AccessorInfo *Connection::FindAccessorInfo(const IResource *res) {
  auto it = accessors_.find(res);
  if (it == accessors_.end()) {
    AccessorInfo *ai = new AccessorInfo();
    accessors_[res] = ai;
    return ai;
  }
  return it->second;
}

}  // namespace writer
}  // namespace iroha
