#include "writer/names.h"

#include "iroha/i_design.h"

namespace iroha {
namespace writer {

Names::Names(Names *parent) : parent_(parent) {
}

Names::~Names() {
}

void Names::ReservePrefix(const string &prefix) {
  prefixes_.insert(prefix);
}

void Names::ReserveGlobalName(const string &name) {
  if (parent_ == nullptr) {
    reserved_names_.insert(name);
  } else {
    parent_->ReserveGlobalName(name);
  }
}

bool Names::IsReservedName(const string &name) {
  bool reserved = reserved_names_.find(name) != reserved_names_.end();
  if (parent_ != nullptr) {
    reserved |= parent_->IsReservedName(name);
  }
  return reserved;
}

bool Names::IsReservedPrefix(const string &prefix) {
  bool reserved = prefixes_.find(prefix) != prefixes_.end();
  if (parent_ != nullptr) {
    reserved |= parent_->IsReservedPrefix(prefix);
  }
  return reserved;
}

void Names::AssignRegNames(const IModule *mod) {
  for (auto *tab : mod->tables_) {
    for (auto *reg : tab->registers_) {
      if (reg->IsConst()) {
	continue;
      }
      reg_names_[reg] = GetName(reg->GetName(), "r_");
    }
  }
}

string Names::GetRegName(const IRegister &reg) {
  return reg_names_[&reg];
}

string Names::GetName(const string &raw_name, const string &type_prefix) {
  string prefix = GetPrefix(raw_name);
  string additional_prefix;
  if (prefix.empty() || IsReservedName(raw_name) ||
      IsReservedPrefix(prefix)) {
    additional_prefix = type_prefix;
  }
  string seq;
  int seq_num = 0;
  while (IsReservedName(additional_prefix + seq + raw_name)) {
    // Prefix is not sufficient. Try to add a sequence number too.
    // e.g. prefix_1_origname
    seq = Util::Itoa(seq_num) + "_";
    seq_num++;
  }
  return additional_prefix + seq + raw_name;
}

string Names::GetPrefix(const string &s) {
  vector<string> tokens;
  Util::SplitStringUsing(s, "_", &tokens);
  if (tokens.size() < 2) {
    // Empty or no prefix.
    return "";
  }
  return tokens[0];
}

Names *Names::NewChildNames() {
  Names *names = new Names(this);
  child_names_.push_back(unique_ptr<Names>(names));
  return names;
}

}  // namespace writer
}  // namespace iroha
