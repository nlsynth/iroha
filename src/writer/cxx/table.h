// -*- C++ -*-
#ifndef _writer_cxx_table_h_
#define _writer_cxx_table_h_

#include "writer/cxx/common.h"

namespace iroha {
namespace writer {
namespace cxx {

class Table {
public:
  Table(const ITable *i_tab);
  ~Table();

  void Build();
  void Write(ostream &os);
  ClassWriter *GetClassWriter();
  string GetTableName();

private:
  void BuildConstructor();
  void BuildDispatcher();

  const ITable *i_tab_;
  unique_ptr<ClassWriter> class_writer_;
  vector<State *> states_;
  string name_;
};

}  // namespace cxx
}  // namespace writer
}  // namespace iroha

#endif  // _writer_cxx_table_h_
