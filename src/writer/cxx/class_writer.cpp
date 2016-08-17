#include "writer/cxx/class_writer.h"

namespace iroha {
namespace writer {
namespace cxx {

ClassMember::ClassMember() : isMethod_(false) {
}

ClassWriter::ClassWriter(const string &name, const string &base)
  : name_(name), base_(base) {
}

void ClassWriter::AddMethod(const string &name, const string &type) {
  ClassMember member;
  member.isMethod_ = true;
  member.type_ = type;
  members_.push_back(member);
}

void ClassWriter::Write(ostream &os) {
  os << "class " << name_;
  if (!base_.empty()) {
    os << " : public " << base_;
  }
  os << " {\npublic:\n";
  for (auto &m : members_) {
    if (m.isMethod_) {
      os << "  " << m.type_ << " " << m.name_ << "() {}\n";
    }
  }
  os << "};\n\n";
}

}  // namespace cxx
}  // namespace writer
}  // namespace iroha
