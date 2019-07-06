// -*- C++ -*-
// Code for DOT (Graphviz) format.
#ifndef _iroha_dot_h_
#define _iroha_dot_h_

#include "iroha/common.h"

#include <map>

namespace iroha {

class Cluster {
public:
  Cluster(const string &name) : name_(name), parent_(nullptr) {
  }

  const string &GetName() const {
    return name_;
  }

  void SetParent(Cluster *parent) {
    parent_ = parent;
  }

  Cluster *GetParent() const {
    return parent_;
  }

private:
  string name_;
  Cluster *parent_;
};

class Node {
public:
  Node(const string &name) : name_(name), sink_(nullptr), cluster_(nullptr) {
  }

  const string &GetName() const {
    return name_;
  }

  Node *GetSink() const {
    return sink_;
  }

  void SetSink(Node *sink) {
    sink_ = sink;
  }

  void SetCluster(Cluster *cluster) {
    cluster_ = cluster;
  }

  Cluster *GetCluster() const {
    return cluster_;
  }

private:
  string name_;
  Node *sink_;
  Cluster *cluster_;
};

class Dot {
public:
  ~Dot();

  void Write(const string &fn);
  void Output(ostream &os);

  Node *GetNode(const string &s);
  Cluster *GetCluster(const string &s);

private:

  unique_ptr<ostream> ofs_;
  std::map<string, Node *> nodes_;
  std::map<string, Cluster *> clusters_;
};

}  // namespace iroha

#endif // _iroha_dot_h_

