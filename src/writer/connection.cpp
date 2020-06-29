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
  // ext-input, ext-output
  ProcessExtIOAccessors(tab);
  // study
  ProcessStudyAccessors(tab);
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
    if (resource::IsSharedRegExtWriter(*rc)) {
      ai->shared_reg_ext_writers_.push_back(res);
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
	resource::IsAxiSlavePort(*rc) ||
	resource::IsSramIf(*rc)) {
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

void Connection::ProcessExtIOAccessors(ITable *tab) {
  for (IResource *res : tab->resources_) {
    IResource *p = res->GetParentResource();
    if (p == nullptr) {
      continue;
    }
    auto *rc = p->GetClass();
    if (resource::IsExtInput(*rc)) {
      auto *ai = FindAccessorInfo(p);
      ai->ext_input_accessors_.push_back(res);
    }
    if (resource::IsExtOutput(*rc)) {
      auto *ai = FindAccessorInfo(p);
      ai->ext_output_accessors_.push_back(res);
    }
  }
}

void Connection::ProcessStudyAccessors(ITable *tab) {
  for (IResource *res : tab->resources_) {
    IResource *p = res->GetParentResource();
    if (p == nullptr ||
	!resource::IsStudy(*(p->GetClass()))) {
      continue;
    }
    auto *ai = FindAccessorInfo(p);
    ai->study_accessors_.push_back(res);
  }
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

const vector<IResource *> &Connection::GetSharedRegExtWriters(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->shared_reg_ext_writers_;
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

const vector<IResource *> &Connection::GetExtInputAccessors(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->ext_input_accessors_;
}

const vector<IResource *> &Connection::GetExtOutputAccessors(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->ext_output_accessors_;
}

const vector<IResource *> &Connection::GetStudyAccessors(const IResource *res) const {
  const auto *ai = GetAccessorInfo(res);
  return ai->study_accessors_;
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
