// -*- C++ -*-
#ifndef _writer_writer_h_
#define _writer_writer_h_

#include "iroha/writer_api.h"

namespace iroha {

class IDesign;
class IModule;
class IState;
class ITable;

class Writer : public WriterAPI {
public:
  Writer(const IDesign *design);
  virtual void Write(const string &fn) override;

private:
  void WriteModule(const IModule *mod);
  void WriteTable(const ITable *tab);
  void WriteState(const IState *st);
  
  const IDesign *design_;
  ostream *os_;
};
  
}  // namespace iroha

#endif  // _writer_writer_h_
