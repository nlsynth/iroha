// Code for DOT (Graphviz) format.
#include "iroha/dot/dot.h"

#include "iroha/stl_util.h"

#include <fstream>
#include <memory>

using namespace std;

namespace iroha {
namespace dot {

Edge::Edge(int id) : id_(id), dotted_(false) {
}

int Edge::GetId() const {
  return id_;
}

void Edge::SetLabel(const string &label) {
  label_ = label;
}

const string &Edge::GetLabel() const {
  return label_;
}

void Edge::SetDotted(bool dotted) {
  dotted_ = dotted;
}

bool Edge::GetDotted() const {
  return dotted_;
}

Cluster::Cluster(const string &name)
  : name_(name), parent_(nullptr) {
}

const string &Cluster::GetName() const {
  return name_;
}

const string &Cluster::GetLabel() const {
  return label_;
}

void Cluster::SetLabel(const string &label) {
  label_ = label;
}

Edge *Cluster::AddSink(Dot *dot, Cluster *sink) {
  Edge *e = dot->NewEdge();
  sinks_[e->GetId()] = sink;
  return e;
}

const std::map<int, Cluster *> &Cluster::GetSinks() const {
  return sinks_;
}

void Cluster::SetParent(Cluster *parent) {
  parent_ = parent;
}

Cluster *Cluster::GetParent() {
  return parent_;
}

Node::Node(const string &name)
  : name_(name), sink_node_(nullptr), sink_cluster_(nullptr),
    cluster_(nullptr), visible_(true) {
}

const string &Node::GetName() const {
  return name_;
}

const string &Node::GetLabel() const {
  return label_;
}

void Node::SetLabel(const string &label) {
  label_ = label;
}

Node *Node::GetSinkNode() const {
  return sink_node_;
}

void Node::SetSinkNode(Node *sink_node) {
  sink_node_ = sink_node;
}

Cluster *Node::GetSinkCluster() const {
  return sink_cluster_;
}

void Node::SetSinkCluster(Cluster *sink_cluster) {
  sink_cluster_ = sink_cluster;
}

void Node::SetCluster(Cluster *cluster) {
  cluster_ = cluster;
}

Cluster *Node::GetCluster() const {
  return cluster_;
}

void Node::SetVisible(bool visible) {
  visible_ = visible;
}

bool Node::GetVisible() const {
  return visible_;
}

Dot::~Dot() {
  STLDeleteSecondElements(&nodes_);
  STLDeleteSecondElements(&clusters_);
  STLDeleteSecondElements(&edges_);
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
    if (sc == nullptr) {
      continue;
    }
    auto &sinks = sc->GetSinks();
    for (auto jt : sinks) {
      Edge *edge = GetEdge(jt.first);
      OutputClusterLink(sc, jt.second, edge, os);
    }
  }
  os << "}\n";
}

void Dot::OutputClusterLink(Cluster *sc, Cluster *pc, Edge *edge,
			    ostream &os) {
  if (pc == nullptr) {
    return;
  }
  Node *sn = cluster_to_one_node_[sc];
  Node *pn = cluster_to_one_node_[pc];
  if (sn == nullptr || pn == nullptr) {
    // Given clusters don't have a node.
    return;
  }
  os << "  " << sn->GetName() << " -> " << pn->GetName()
     << " [";
  string label = edge->GetLabel();
  if (!label.empty()) {
    os << "label=\"" + label + "\" ";
  }
  if (edge->GetDotted()) {
    os << "style=\"dotted\" ";
  }
  os << "ltail=cluster_" << sc->GetName()
     << " lhead=cluster_" << pc->GetName() << "]\n";
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

Edge *Dot::NewEdge() {
  int id = edges_.size() + 1;
  Edge *e = new Edge(id);
  edges_[id] = e;
  return e;
}

Edge *Dot::GetEdge(int id) const {
  auto it = edges_.find(id);
  if (it == edges_.end()) {
    return nullptr;
  }
  return it->second;
}

void Dot::Write(const string &fn) {
  ofs_.reset(new ofstream(fn));
  Output(*ofs_);
}

}  // namespace dot
}  // namespace iroha
