// -*- C++ -*-
#ifndef _writer_verilog_shared_reg_h_
#define _writer_verilog_shared_reg_h_

#include "writer/verilog/resource.h"

namespace iroha {
namespace writer {
namespace verilog {

class SharedReg : public Resource {
public:
  SharedReg(const IResource &res, const Table &table);

  virtual void BuildResource() override;
  virtual void BuildInsn(IInsn *insn, State *st) override;

  static string RegName(const IResource &res);
  static string WriterName(const IResource &res);
  static string WriterEnName(const IResource &res);

  // Reader
  static void BuildReaderPorts(const ResourceConnectionInfo &pi,
			       Ports *ports);
  static void BuildReaderChildWire(const ResourceConnectionInfo &pi,
				   ostream &os);
  static void BuildReaderRootWire(const ResourceConnectionInfo &pi,
				  Module *module);
  // Writer
  static void BuildWriterPorts(const ResourceConnectionInfo &pi,
			       Ports *ports);
  static void BuildWriterChildWire(const ResourceConnectionInfo &pi,
				   ostream &os);
  static void BuildWriterRootWire(const ResourceConnectionInfo &pi,
				  Module *module);
  static void BuildRootWire(const ResourceConnectionInfo &pi,
			    bool is_write,
			    Module *module);

private:
  static void AddChildWire(IResource *res, bool is_write, ostream &os);
  void BuildSharedRegResource();

  int width_;
  bool has_default_output_value_;
  int default_output_value_;
  const vector<IResource *> *writers_;
  bool need_write_arbitration_;
};

}  // namespace verilog
}  // namespace writer
}  // namespace iroha

#endif  // _writer_verilog_shared_reg_h_
