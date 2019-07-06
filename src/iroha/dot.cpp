// Code for DOT (Graphviz) format.
#include "iroha/dot.h"

#include "iroha/stl_util.h"

#include <fstream>
#include <memory>

using namespace std;

namespace iroha {

Dot::~Dot() {
  STLDeleteSecondElements(&nodes_);
  STLDeleteSecondElements(&clusters_);
}

void Dot::Output(ostream &os) {
  os << "digraph g {\n";
  for (auto it : clusters_) {
    Cluster *c = it.second;
    os << "  subgraph cluster_" << c->GetName() << " {\n"
       << "    label = \"" << c->GetName() << "\"\n";
    for (auto jt : nodes_) {
      Node *n = jt.second;
      if (n->GetCluster() == c) {
	os << "    " << n->GetName() << "\n";
      }
    }
    os << "  }\n";
  }
  for (auto it : nodes_) {
    Node *n = it.second;
    if (n->GetCluster() == nullptr) {
      os << "  " << n->GetName() << "\n";
    }
  }
  for (auto it : nodes_) {
    Node *n = it.second;
    Node *s = n->GetSink();
    if (s != nullptr) {
      os << "  " << n->GetName() << " -> " << s->GetName() << "\n";
    }
  }
  os << "}\n";
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

}  // namespace iroha
