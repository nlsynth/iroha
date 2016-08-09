// -*- C++ -*-
#ifndef _writer_cxx_cxx_writer_h_
#define _writer_cxx_cxx_writer_h_

#include "writer/cxx/common.h"

namespace iroha {
namespace writer {
namespace cxx {

class CxxWriter {
public:
  CxxWriter(const IDesign *design, ostream &os);

  void Write();

private:
  const IDesign *design_;
  ostream &os_;
};

}  // namespace cxx
}  // namespace writer
}  // namespace iroha

#endif  // _writer_cxx_cxx_writer_h_
