// -*- C++ -*-
#ifndef _iroha_writer_api_h_
#define _iroha_writer_api_h_

#include "iroha/common.h"

namespace iroha {

class WriterAPI {
public:
  virtual ~WriterAPI();
  virtual bool Write(const string &fn) = 0;
  virtual bool SetLanguage(const string &lang) = 0;
  virtual void OutputShellModule(bool b, bool self_contained, bool vcd) = 0;
  virtual void SetOutputConfig(const string &root, const string &marker,
			       bool debug) = 0;
};

}  // namespace iroha

#endif  // _iroha_writer_api_h_
