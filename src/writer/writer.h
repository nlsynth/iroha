// -*- C++ -*-
#ifndef _writer_writer_h_
#define _writer_writer_h_

#include "iroha/writer_api.h"

namespace iroha {

class Writer : public WriterAPI {
public:
  Writer(const IDesign *design);

  virtual bool Write(const string &fn) override;
  virtual bool SetLanguage(const string &lang) override;

  const IDesign *design_;
  string language_;
};
  
}  // namespace iroha

#endif  // _writer_writer_h_
