#include "iroha/logging.h"

#include <stdlib.h>
#include <sstream>

namespace iroha {

static stringstream ss;

ostream &Logger::GetStream(LogSeverity sev) {
  return ss;
}

void Logger::Finalize(LogSeverity sev, const char *fn, int line) {
  if (sev == LOG_NONE) {
    ss.str("");
    return;
  }
  cout << fn << ":" << line << ":" << ss.str() << "\n";
  ss.str("");
  if (sev == FATAL) {
    abort();
  }
}

LogFinalizer::LogFinalizer(LogSeverity sev, const char *fn, int line)
  : sev_(sev), fn_(fn), line_(line) {
}

LogFinalizer::~LogFinalizer() {
  Logger::Finalize(sev_, fn_, line_);
}

}  // namespace iroha
