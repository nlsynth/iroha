#include "writer/verilog/wire/names.h"

#include "iroha/i_design.h"

namespace iroha {
namespace writer {
namespace verilog {
namespace wire {

string Names::AccessorName(const string &resource_name, const IResource *res) {
  if (res == nullptr) {
    return resource_name;
  }
  return resource_name + "_" + AccessorResourceName(res);
}

string Names::AccessorResourceName(const IResource *res) {
  if (res == nullptr) {
    return "";
  }
  ITable *tab = res->GetTable();
  IModule *mod = tab->GetModule();
  return Util::Itoa(mod->GetId()) + "_" + Util::Itoa(tab->GetId()) + "_"
    + Util::Itoa(res->GetId());
}

string Names::AccessorSignalBase(const string &resource_name,
				 const IResource *res, const char *name) {
  string s = AccessorName(resource_name, res);
  if (name != nullptr) {
    s += "_" + string(name);
  }
  return s;
}

string Names::AccessorWire(const string &resource_name, const IResource *res,
			   const char *name) {
  return AccessorSignalBase(resource_name, res, name) + "_wire";
}

string Names::ResourceSignalBase(const string &resource_name,
				 const char *name) {
  return AccessorSignalBase(resource_name, nullptr, name);
}

string Names::ResourceWire(const string &resource_name, const char *name) {
  return ResourceSignalBase(resource_name, name) + "_wire";
}

}  // namespace wire
}  // namespace verilog
}  // namespace writer
}  // namespace iroha
