// placeholder for experiments.
#include "opt/study.h"

namespace iroha {
namespace opt {

Study::~Study() {
}

Phase *Study::Create() {
  return new Study();
}

}  // namespace opt
}  // namespace iroha

