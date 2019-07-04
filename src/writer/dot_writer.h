// -*- C++ -*-
#ifndef _writer_dot_writer_h_
#define _writer_dot_writer_h_

#include "iroha/common.h"

namespace iroha {
namespace writer {

class DotWriter {
public:
  DotWriter(const IDesign *design, ostream &os);

  void Write();

private:
  ostream &os_;
};

}  // namespace writer
}  // namespace iroha

#endif
