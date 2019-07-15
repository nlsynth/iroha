// -*- C++ -*-
// Code for DOT (Graphviz) format.
#ifndef _iroha_dot_dot_h_
#define _iroha_dot_dot_h_

#include "iroha/common.h"

#include <map>

namespace iroha {
namespace dot {

class Dot;

class Edge {
public:
  Edge(int id);

  int GetId() const;
  void SetLabel(const string &label);
  const string &GetLabel() const;
  void SetDotted(bool dotted);
  bool GetDotted() const;

private:
  int id_;
  string label_;
  bool dotted_;
};

class Cluster {
public:
  Cluster(const string &name);

  const string &GetName() const;
  const string &GetLabel() const;
  void SetLabel(const string &label);
  Edge *AddSink(Dot *dot, Cluster *sink);
  const std::map<int, Cluster *> &GetSinks() const;
  void SetParent(Cluster *parent);
  Cluster *GetParent();

private:
  string name_;
  string label_;
  std::map<int, Cluster *> sinks_;
  Cluster *parent_;
};

class Node {
public:
  Node(const string &name);

  const string &GetName() const;
  const string &GetLabel() const;
  void SetLabel(const string &label);
  Node *GetSinkNode() const;
  void SetSinkNode(Node *sink_node);
  Cluster *GetSinkCluster() const;
  void SetSinkCluster(Cluster *sink_cluster);
  void SetCluster(Cluster *cluster);
  Cluster *GetCluster() const;
  void SetVisible(bool visible);
  bool GetVisible() const;

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
  Edge *NewEdge();
  Edge *GetEdge(int id) const;

private:
  void OutputCluster(Cluster *cl, ostream &os);
  void OutputClusterLink(Cluster *sc, Cluster *pc, Edge *edge, ostream &os);
  void OutputNode(Node *node, ostream &os);
  void BuildTree();

  unique_ptr<ostream> ofs_;
  std::map<int, Edge *> edges_;
  std::map<string, Node *> nodes_;
  std::map<string, Cluster *> clusters_;
  std::map<Cluster *, Node *> cluster_to_one_node_;
  std::map<Cluster *, vector<Cluster *> > child_clusters_;
};

}  // namespace dot
}  // namespace iroha

#endif // _iroha_dot_dot_h_
