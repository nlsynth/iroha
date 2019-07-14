// Code for DOT (Graphviz) format.
#include "iroha/dot/dot.h"

#include "iroha/stl_util.h"

#include <fstream>
#include <memory>

using namespace std;

namespace iroha {
namespace dot {

Cluster::Cluster(const string &name)
  : name_(name), sink_(nullptr), parent_(nullptr) {
}

Node::Node(const string &name)
  : name_(name), sink_node_(nullptr), sink_cluster_(nullptr),
    cluster_(nullptr), visible_(true) {
}

Dot::~Dot() {
  STLDeleteSecondElements(&nodes_);
  STLDeleteSecondElements(&clusters_);
}

void Dot::Output(ostream &os) {
  BuildTree();
  os << "digraph g {\n"
     << "  compound = true;\n";
  // Subgraphs from each root.
  for (auto it : clusters_) {
    Cluster *cl = it.second;
    if (cl->GetParent() != nullptr) {
      continue;
    }
    OutputCluster(cl, os);
  }
  // Independent node decls and links.
  for (auto it : nodes_) {
    Node *n = it.second;
    if (n->GetCluster() == nullptr) {
      OutputNode(n, os);
    }
  }
  for (auto it : nodes_) {
    Node *n = it.second;
    Node *sn = n->GetSinkNode();
    if (sn != nullptr) {
      os << "  " << n->GetName() << " -> " << sn->GetName() << "\n";
    }
    Cluster *sc = n->GetSinkCluster();
    if (sc != nullptr) {
      Node *scn = cluster_to_one_node_[sc];
      os << "  " << n->GetName() << " -> " << scn->GetName()
	 << " ["
	 << " lhead=cluster_" << sc->GetName() << "]\n";
    }
  }
  // Cluster to cluster link.
  for (auto it : clusters_) {
    Cluster *sc = it.second;
    Cluster *pc = sc->GetSink();
    if (pc == nullptr) {
      continue;
    }
    Node *sn = cluster_to_one_node_[sc];
    Node *pn = cluster_to_one_node_[pc];
    if (sn == nullptr || pn == nullptr) {
      // Given clusters don't have a node.
      continue;
    }
    os << "  " << sn->GetName() << " -> " << pn->GetName()
       << " [ltail=cluster_" << sc->GetName()
       << " lhead=cluster_" << pc->GetName() << "]\n";
  }
  os << "}\n";
}

void Dot::BuildTree() {
  // Pick one node for each cluster to generate links between clusters.
  for (auto it : nodes_) {
    Node *n = it.second;
    Cluster *cl = n->GetCluster();
    if (cl != nullptr) {
      cluster_to_one_node_[cl] = n;
    }
  }
  // Build parent to children map.
  for (auto it : clusters_) {
    Cluster *cl = it.second;
    Cluster *pc = cl->GetParent();
    if (pc == nullptr) {
      continue;
    }
    child_clusters_[pc].push_back(cl);
  }
}

void Dot::OutputCluster(Cluster *cl, ostream &os) {
  string l = cl->GetLabel();
  if (l.empty()) {
    l = cl->GetName();
  }
  os << "  subgraph cluster_" << cl->GetName() << " {\n"
     << "    label = \"" << l << "\"\n";
  for (Cluster *cc : child_clusters_[cl]) {
    OutputCluster(cc, os);
  }
  for (auto it : nodes_) {
    Node *n = it.second;
    if (n->GetCluster() == cl) {
      OutputNode(n, os);
    }
  }
  os << "  }\n";
}

Node *Dot::GetNode(const string &name) {
  auto it = nodes_.find(name);
  if (it != nodes_.end()) {
    return it->second;
  }
  Node *n = new Node(name);
  nodes_[name] = n;
  return n;
}

void Dot::OutputNode(Node *node, ostream &os) {
  os << "    " << node->GetName();
  if (!node->GetVisible()) {
    os << " [label=\"\", style=invis]";
  } else {
    string l = node->GetLabel();
    if (!l.empty()) {
      os << " [label=\"" << node->GetLabel() << "\"]";
    }
  }
  os << "\n";
}

Cluster *Dot::GetCluster(const string &name) {
  auto it = clusters_.find(name);
  if (it != clusters_.end()) {
    return it->second;
  }
  Cluster *c = new Cluster(name);
  clusters_[name] = c;
  return c;
}

void Dot::Write(const string &fn) {
  ofs_.reset(new ofstream(fn));
  Output(*ofs_);
}

}  // namespace dot
}  // namespace iroha
