// -*- C++ -*-
#ifndef _writer_cxx_module_h_
#define _writer_cxx_module_h_

#include "writer/cxx/common.h"

namespace iroha {
namespace writer {
namespace cxx {

class Module {
public:
  Module(const IModule *i_mod);
  ~Module();

  void Build();
  void Write(ostream &os);
  string GetName();

private:
  const IModule *i_mod_;
  unique_ptr<ClassWriter> class_writer_;
  vector<Table *> tables_;
};

}  // namespace cxx
}  // namespace writer
}  // namespace iroha

#endif  // _writer_cxx_module_h_
