#include "opt/array_to_mem.h"

#include <map>

#include "design/util.h"
#include "iroha/i_design.h"
#include "iroha/resource_class.h"
#include "iroha/resource_params.h"

namespace iroha {
namespace opt {

static const string name("array_to_mem");

ArrayToMem::~ArrayToMem() {
}

Phase *ArrayToMem::Create() {
  return new ArrayToMem();
}

bool ArrayToMem::ApplyForTable(ITable *table) {
  map<IResource *, IResource *> array_to_mem;
  auto copied_res = table->resources_;
  for (IResource *res : copied_res) {
    if (res->GetClass()->GetName() == resource::kArray) {
      IResource *mem_res = GetResource(res);
      array_to_mem[res] = mem_res;
    }
  }
  // TODO(yt76): Implement the rest.
  return true;
}

IResource *ArrayToMem::GetResource(IResource *array_res) {
  ITable *table = array_res->GetTable();
  for (auto *res : table->resources_) {
    if (res->GetArray() == array_res->GetArray() &&
	res->GetClass()->GetName() == resource::kMapped) {
      auto *param = res->GetParams();
      if (param->GetMappedName() == "mem") {
	return res;
      }
    }
  }
  IResource *res = DesignUtil::CreateResource(table, resource::kMapped);
  auto *param = res->GetParams();
  param->SetMappedName("mem");
  res->SetArray(array_res->GetArray());
  return res;
}

}  // namespace opt
}  // namespace iroha
