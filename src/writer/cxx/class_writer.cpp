#include "writer/cxx/class_writer.h"

#include "iroha/stl_util.h"

namespace iroha {
namespace writer {
namespace cxx {

ClassMember::ClassMember() : isMethod_(false) {
}

ClassMember::~ClassMember() {
}

ClassWriter::ClassWriter(const string &name, const string &base)
  : name_(name), base_(base) {
}

ClassWriter::~ClassWriter() {
  STLDeleteValues(&members_);
}

ClassMember *ClassWriter::AddMethod(const string &name, const string &type) {
  return AddMember(true, name, type);
}

void ClassWriter::AddVariable(const string &name, const string &type) {
  AddMember(false, name, type);
}

ClassMember *ClassWriter::AddMember(bool isMethod, const string &name, const string &type) {
  ClassMember *member = new ClassMember;
  member->name_ = name;
  member->isMethod_ = isMethod;
  member->type_ = type;
  members_.push_back(member);
  return member;
}

const string &ClassWriter::GetName() const {
  return name_;
}

void ClassWriter::Write(ostream &os) {
  os << "class " << name_;
  if (!base_.empty()) {
    os << " : public " << base_;
  }
  os << " {\npublic:\n";
  for (auto *m : members_) {
    if (m->isMethod_) {
      os << "  ";
      if (!m->type_.empty()) {
	os << m->type_ << " ";
      }
      os << m->name_ << "() {\n";
      os << m->body_;
      os << "  }\n";
    } else {
      os << "  " << m->type_ << " " << m->name_ << ";\n";
    }
  }
  os << "};\n\n";
}

ClassMember *ClassWriter::FindConstructor() {
  for (auto *m : members_) {
    if (m->name_ == name_) {
      return m;
    }
  }
  return nullptr;
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
