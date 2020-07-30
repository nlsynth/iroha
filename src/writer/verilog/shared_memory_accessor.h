// -*- C++ -*-
#ifndef _writer_verilog_shared_memory_accessor_h_
#define _writer_verilog_shared_memory_accessor_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedMemoryAccessor : public Resource {
public:
  SharedMemoryAccessor(const IResource &res, const Table &table);
  virtual ~SharedMemoryAccessor() {};

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static void BuildAccessInsn(IInsn *insn, State *st, const IResource &res,
			      const Table &tab);

  // gen_reg determines if addr/wdata/req/wen are driven from this module.
  // generates wires and let them driven by dmac.
  static void BuildMemoryAccessorResource(const Resource &accessor,
					  bool do_write, bool gen_reg,
					  const IResource *mem);

  static string AddrSrc(const IResource &accessor);
  static string ReqSrc(const IResource &accessor);
  static string WDataSrc(const IResource &accessor);
  static string WEnSrc(const IResource &accessor);

private:
  static const IResource *GetMem(const IResource &accessor);
  static string SrcName(const IResource &accessor, const string &name);
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_memory_accessor_h_
