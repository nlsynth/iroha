#include "platform/platform_db.h"

namespace iroha {
namespace platform {

PlatformDB::PlatformDB(IPlatform *platform) : platform_(platform) {
}

}  // namespace platform
}  // namespace iroha
