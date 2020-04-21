#include "opt/array_split_rdata.h"

namespace iroha {
namespace opt {

ArraySplitRData::~ArraySplitRData() {
}

Phase *ArraySplitRData::Create() {
  return new ArraySplitRData();
}

}  // namespace opt
}  // namespace iroha
