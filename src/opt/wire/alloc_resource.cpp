// WIP. This inherits implementation from wire_insn phase.
#include "opt/wire/alloc_resource.h"

#include "design/resource_attr.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace opt {
namespace wire {

AllocResourcePhase::~AllocResourcePhase() {
}

Phase *AllocResourcePhase::Create() {
  return new AllocResourcePhase();
}

bool AllocResourcePhase::ApplyForTable(const string &key, ITable *table) {
  AllocResource alloc_resource(table, annotation_);
  return alloc_resource.Perform();
}

AllocResource::AllocResource(ITable *table, DebugAnnotation *annotation)
  : WireInsn(table, annotation) {
}

AllocResource::~AllocResource() {
}

bool AllocResource::CanUseResourceInState(IState *st, IResource *resource,
					  MoveStrategy *ms) {
  bool ok = WireInsn::CanUseResourceInState(st, resource, ms);
  if (ok) {
    // Simply ok.
    return true;
  }
  if (!ResourceAttr::IsDuplicatableResource(resource)) {
    return false;
  }
  ms->use_same_resource = false;
  return true;
}

IInsn *AllocResource::MayCopyInsnForState(IState *st, IInsn *insn,
					  MoveStrategy *ms) {
  // See if the same resource is usable.
  IResource *res = insn->GetResource();
  bool used = false;
  for (IInsn *ii : st->insns_) {
    if (ii->GetResource() == res) {
      used = true;
    }
  }
  if (!used) {
    return insn;
  }
  // Same resource is not usable.
  IResource *new_res = FindCompatibleResource(st, res);
  if (new_res == nullptr) {
    new_res = CopyResource(res);
  }
  // Copies insn with the new resource.
  IInsn *new_insn = new IInsn(new_res);
  new_insn->inputs_ = insn->inputs_;
  new_insn->outputs_ = insn->outputs_;
  new_insn->target_states_ = insn->target_states_;
  // Assuming this is a simple insn and doesn't have dependencies.
  new_insn->depending_insns_ = insn->depending_insns_;
  st->insns_.push_back(new_insn);
  return new_insn;
}

IResource *AllocResource::CopyResource(IResource *src) {
  ITable *tab = src->GetTable();
  IResource *res = new IResource(tab, src->GetClass());
  res->input_types_ = src->input_types_;
  res->output_types_ = src->output_types_;
  tab->resources_.push_back(res);
  return res;
}

IResource *AllocResource::FindCompatibleResource(IState *st, IResource *res) {
  set<IResource *> used_res;
  for (IInsn *ii : st->insns_) {
    used_res.insert(ii->GetResource());
  }
  for (IResource *r : st->GetTable()->resources_) {
    if (used_res.find(r) != used_res.end()) {
      continue;
    }
    if (IsCompatibleResource(res, r)) {
      return r;
    }
  }
  return nullptr;
}

bool AllocResource::IsCompatibleResource(IResource *orig, IResource *res) {
  IResourceClass &orc = *(orig->GetClass());
  IResourceClass &rc = *(res->GetClass());
  if (orc.GetName() != rc.GetName()) {
    return false;
  }
  if (resource::IsExtCombinational(rc)) {
    auto *op = orig->GetParams();
    auto *rp = res->GetParams();
    if (op->GetEmbeddedModuleFileName() ==
	rp->GetEmbeddedModuleFileName() &&
	op->GetEmbeddedModuleName() ==
	rp->GetEmbeddedModuleName()) {
      return true;
    }
  }
  // TODO: Implementation for add, sub, mul and so on.
  return false;
}

}  // namespace wire
}  // namespace opt
}  // namespace iroha
