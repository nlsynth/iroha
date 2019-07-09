#include "writer/dot_writer.h"

#include "iroha/dot.h"
#include "iroha/i_design.h"

namespace iroha {
namespace writer {

DotWriter::DotWriter(const IDesign *design, ostream &os)
  : design_(design), os_(os) {
}

DotWriter::~DotWriter() {
}

void DotWriter::Write() {
  dot_.reset(new Dot);
  for (auto *mod : design_->modules_) {
    Cluster *mod_cl = dot_->GetCluster(ModuleName(*mod));
    mod_cl->SetLabel(mod->GetName());
    mod_to_cluster_[mod] = mod_cl;
    for (auto *tab : mod->tables_) {
      Cluster *tab_cl = dot_->GetCluster(TableName(*tab));
      tab_to_cluster_[tab] = tab_cl;
      for (IResource *res : tab->resources_) {
	Node *n = dot_->GetNode(ResourceName(*res));
	res_to_node_[res] = n;
      }
    }
  }
  for (auto *mod : design_->modules_) {
    WriteModule(*mod);
  }
  dot_->Output(os_);
}

void DotWriter::WriteModule(const IModule &mod) {
  Cluster *cl = mod_to_cluster_[&mod];
  Node *n = dot_->GetNode("dummy_" + mod.GetName());
  n->SetVisible(false);
  n->SetCluster(cl);
  IModule *parent = mod.GetParentModule();
  if (parent != nullptr) {
    Cluster *sink_cl = mod_to_cluster_[parent];
    cl->SetSink(sink_cl);
  }
  for (auto *tab : mod.tables_) {
    WriteTable(*tab);
  }
}

void DotWriter::WriteTable(const ITable &tab) {
  Cluster *mod_cl = mod_to_cluster_[tab.GetModule()];
  Cluster *tab_cl = dot_->GetCluster(TableName(tab));
  tab_cl->SetParent(mod_cl);
  for (IResource *res : tab.resources_) {
    Node *n = dot_->GetNode(ResourceName(*res));
    n->SetLabel(res->GetClass()->GetName() + "_" + Util::Itoa(res->GetId()));
    n->SetCluster(tab_cl);
    IResource *pres = res->GetParentResource();
    if (pres != nullptr) {
      n->SetSinkNode(res_to_node_[pres]);
    }
    ITable *callee = res->GetCalleeTable();
    if (callee != nullptr) {
      Cluster *callee_cl = tab_to_cluster_[callee];
      n->SetSinkCluster(callee_cl);
    }
  }
}

string DotWriter::ModuleName(const IModule &mod) {
  return "mod_" + Util::Itoa(mod.GetId());
}

string DotWriter::TableName(const ITable &tab) {
  return "tab_" + Util::Itoa(tab.GetModule()->GetId())
    + "_" + Util::Itoa(tab.GetId());
}

string DotWriter::ResourceName(const IResource &res) {
  return "res_" + Util::Itoa(res.GetTable()->GetModule()->GetId())
    + "_" + Util::Itoa(res.GetTable()->GetId())
    + "_" + Util::Itoa(res.GetId());
}

}  // namespace writer
}  // namespace iroha
