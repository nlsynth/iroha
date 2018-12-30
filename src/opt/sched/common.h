// -*- C++ -*-
#ifndef _opt_sched_common_h_
#define _opt_sched_common_h_

#include "opt/common.h"

namespace iroha {
namespace opt {
namespace sched {

class BBDataPath;
class BBResourceTracker;
class BBWirePlan;
class DataPathSet;
class Explorer;
class PathEdge;
enum PathEdgeType : int;
class PathNode;
class PlanEvaluator;
class ResourceConflictTracker;
class ResourceEntry;
class ResourceShare;
class VirtualResource;
class VirtualResourceSet;
class WirePlan;
class WirePlanSet;

}  // namespace sched
}  // namespace opt
}  // namespace iroha

#endif  // _opt_sched_common_h_
