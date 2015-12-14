// -*- C++ -*-
#ifndef _iroha_logging_h_
#define _iroha_logging_h_

#include "iroha/common.h"

namespace iroha {

enum LogSeverity {
  LOG_NONE,
  INFO,
  FATAL,
};

class Logger {
public:
  static std::ostream &GetStream(LogSeverity sev);
  static void Finalize(LogSeverity sev, const char *fn, int line);
};

class LogFinalizer {
public:
  LogFinalizer(LogSeverity sev, const char *fn, int line);
  ~LogFinalizer();

  void operator&(std::ostream &none) {}

private:
  LogSeverity sev_;
  const char *fn_;
  int line_;
};

#define LOG(s) LogFinalizer(s, __FILE__, __LINE__) & Logger::GetStream(s)
#define CHECK(s) LogFinalizer((s ? LOG_NONE : FATAL), __FILE__, __LINE__) & Logger::GetStream((s ? LOG_NONE : FATAL))

}  // namespace iroha

#endif  // _iroha_logging_h_
