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
  ~ClassMember();

  bool isMethod_;
  string type_;
  string name_;
  string body_;
};

class ClassWriter {
public:
  ClassWriter(const string &name, const string &base);
  ~ClassWriter();

  void Write(ostream &os);
  ClassMember *AddMethod(const string &name, const string &type);
  void AddVariable(const string &name, const string &type);
  const string &GetName() const;
  ClassMember *FindConstructor();

private:
  ClassMember *AddMember(bool isMethod, const string &name, const string &type);

  const string name_;
  const string base_;

  vector<ClassMember *> members_;
};

}  // namespace cxx
}  // namespace writer
}  // namespace iroha

#endif  // _writer_cxx_class_writer_h_
