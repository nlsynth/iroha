#include "builder/platform_builder.h"

#include "builder/design_builder.h"
#include "builder/reader.h"
#include "iroha/i_design.h"
#include "iroha/i_platform.h"

namespace iroha {
namespace builder {

PlatformBuilder::PlatformBuilder(DesignBuilder &design_builder)
  : design_builder_(design_builder) {
}

void PlatformBuilder::BuildPlatform(Exp *e, IDesign *design) {
  IPlatform *platform = new IPlatform(design);
  design->platforms_.push_back(platform);
  for (int i = 1; i < e->Size(); ++i) {
    BuildDefinition(e->vec[i], platform);
  }
}

void PlatformBuilder::BuildDefinition(Exp *e, IPlatform *platform) {
  if (e->GetHead() != "DEF") {
    design_builder_.SetError() << "Only DEF is allowed in PLATFORM";
    return;
  }
  platform::Definition *def = new platform::Definition(platform);
  BuildCondition(e->vec[1], def);
  BuildValue(e->vec[2], def);
  platform->defs_.push_back(def);
}

void PlatformBuilder::BuildCondition(Exp *e, platform::Definition *def) {
  if (e->GetHead() != "COND") {
    design_builder_.SetError() << "COND should come first of DEF";
    return;
  }
  if (e->Size() == 2) {
    def->condition_ = BuildNode(e->vec[1], def);
  } else {
    def->condition_ = BuildConjunctionFrom2nd(e, def);
  }
}

void PlatformBuilder::BuildValue(Exp *e, platform::Definition *def) {
  if (e->GetHead() != "VALUE") {
    design_builder_.SetError() << "VALUE should come first of DEF";
    return;
  }
  if (e->Size() == 2) {
    def->value_ = BuildNode(e->vec[1], def);
  } else {
    def->value_ = BuildConjunctionFrom2nd(e, def);
  }
}

platform::DefNode *PlatformBuilder::BuildNode(Exp *e, platform::Definition *def) {
  platform::DefNode *node = new platform::DefNode(def);
  if (e->atom.str.empty()) {
    for (Exp *child : e->vec) {
      node->nodes_.push_back(BuildNode(child, def));
    }
  } else {
    node->is_atom_ = true;
    node->str_ = e->atom.str;
  }
  return node;
}

platform::DefNode *PlatformBuilder::BuildConjunctionFrom2nd(Exp *e,
							    platform::Definition *def) {
  platform::DefNode *node = BuildNode(e, def);
  // This is a bit hacky to allow implicit AND.
  // build for (COND-OR-VALUE c1 c2 .. cn) and sets AND later.
  node->nodes_[0]->str_ = "AND";
  return node;
}

}  // namespace builder
}  // namespace iroha
