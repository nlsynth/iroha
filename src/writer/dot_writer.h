// -*- C++ -*-
#ifndef _writer_dot_writer_h_
#define _writer_dot_writer_h_

#include "iroha/common.h"

#include <map>

using std::map;

namespace iroha {

class Cluster;
class Dot;
class Node;

namespace writer {

class DotWriter {
public:
  DotWriter(const IDesign *design, ostream &os);
  ~DotWriter();

  void Write();

private:
  void WriteModule(const IModule &mod);
  void WriteTable(const ITable &tab);
  string ModuleName(const IModule &mod);
  string TableName(const ITable &tab);
  string ResourceName(const IResource &res);

  const IDesign *design_;
  ostream &os_;
  std::unique_ptr<Dot> dot_;
  map<const IModule *, Cluster *> mod_to_cluster_;
  map<const ITable *, Cluster *> tab_to_cluster_;
  map<const IResource *, Node *> res_to_node_;
};

}  // namespace writer
}  // namespace iroha

#endif
