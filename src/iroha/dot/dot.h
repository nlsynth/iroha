// -*- C++ -*-
// Code for DOT (Graphviz) format.
#ifndef _iroha_dot_dot_h_
#define _iroha_dot_dot_h_

#include "iroha/common.h"

#include <map>

namespace iroha {
namespace dot {

class Cluster {
public:
  Cluster(const string &name);

  const string &GetName() const {
    return name_;
  }

  const string &GetLabel() const {
    return label_;
  }

  void SetLabel(const string &label) {
    label_ = label;
  }

  void SetSink(Cluster *sink) {
    sink_ = sink;
  }

  Cluster *GetSink() const {
    return sink_;
  }

  void SetParent(Cluster *parent) {
    parent_ = parent;
  }

  Cluster *GetParent() {
    return parent_;
  }

private:
  string name_;
  string label_;
  Cluster *sink_;
  Cluster *parent_;
};

class Node {
public:
  Node(const string &name);

  const string &GetName() const {
    return name_;
  }

  const string &GetLabel() const {
    return label_;
  }

  void SetLabel(const string &label) {
    label_ = label;
  }

  Node *GetSinkNode() const {
    return sink_node_;
  }

  void SetSinkNode(Node *sink_node) {
    sink_node_ = sink_node;
  }

  Cluster *GetSinkCluster() const {
    return sink_cluster_;
  }

  void SetSinkCluster(Cluster *sink_cluster) {
    sink_cluster_ = sink_cluster;
  }

  void SetCluster(Cluster *cluster) {
    cluster_ = cluster;
  }

  Cluster *GetCluster() const {
    return cluster_;
  }

  void SetVisible(bool visible) {
    visible_ = visible;
  }

  bool GetVisible() const {
    return visible_;
  }

private:
  string name_;
  string label_;
  Node *sink_node_;
  Cluster *sink_cluster_;
  Cluster *cluster_;
  bool visible_;
};

class Dot {
public:
  ~Dot();

  void Write(const string &fn);
  void Output(ostream &os);

  Node *GetNode(const string &s);
  Cluster *GetCluster(const string &s);

private:
  void OutputCluster(Cluster *cl, ostream &os);
  void OutputNode(Node *node, ostream &os);
  void BuildTree();

  unique_ptr<ostream> ofs_;
  std::map<string, Node *> nodes_;
  std::map<string, Cluster *> clusters_;
  std::map<Cluster *, Node *> cluster_to_one_node_;
  std::map<Cluster *, vector<Cluster *> > child_clusters_;
};

}  // namespace dot
}  // namespace iroha

#endif // _iroha_dot_dot_h_
