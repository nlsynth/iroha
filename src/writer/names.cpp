#include "writer/names.h"

#include "iroha/i_design.h"

namespace iroha {
namespace writer {

void Names::ReservePrefix(const string &prefix) {
  prefixes_.insert(prefix);
}

string Names::GetName(const IRegister &reg) {
  string name = reg.GetName();
  string prefix = GetPrefix(name);
  if (prefix.empty() || prefixes_.find(prefix) != prefixes_.end()) {
    name = "r_" + name;
  }
  return name;
}

string Names::GetPrefix(string &s) {
  vector<string> tokens;
  Util::SplitStringUsing(s, "_", &tokens);
  if (tokens.size() > 2) {
    // Empty or no prefix.
    return "";
  }
  return tokens[0];
}

}  // namespace writer
}  // namespace iroha
