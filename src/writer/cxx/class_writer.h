// -*- C++ -*-
#ifndef _writer_cxx_class_writer_h_
#define _writer_cxx_class_writer_h_

#include "writer/cxx/common.h"

namespace iroha {
namespace writer {
namespace cxx {

class ClassMember {
public:
  ClassMember();

  bool isMethod_;
  string type_;
  string name_;
};

class ClassWriter {
public:
  ClassWriter(const string &name, const string &base);

  void Write(ostream &os);
  void AddMethod(const string &name, const string &type);

private:
  const string name_;
  const string base_;

  vector<ClassMember> members_;
};

}  // namespace cxx
}  // namespace writer
}  // namespace iroha

#endif  // _writer_cxx_class_writer_h_
