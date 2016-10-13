// -*- C++ -*-
#ifndef _writer_exp_writer_h_
#define _writer_exp_writer_h_

#include "iroha/common.h"

namespace iroha {

class ExpWriter {
public:
  ExpWriter(const IDesign *design, ostream &os);

  void Write();

private:
  void WriteModule(const IModule &mod);
  void WriteTable(const ITable &tab);
  void WriteInitialState(const ITable &tab);
  void WriteState(const IState &st);
  void WriteInsn(const IInsn &insn);
  void WriteRegisters(const ITable &tab);
  void WriteResources(const ITable &tab);
  void WriteResource(const IResource &res);
  void WriteArrayDesc(const IResource &res);
  void WriteForeignRegDesc(const IResource &res);
  void WriteCalleeTaskDesc(const IResource &res);
  void WriteSharedRegDesc(const IResource &res);
  void WriteValue(const IValue &value);
  void WriteValueType(const IValueType &type);
  void WriteInsnParams(const vector<IRegister *> &regs);
  void WriteResourceTypes(const vector<IValueType> &types);
  void WriteResourceParams(const ResourceParams &params, const char *indent);
  void WriteChannel(const IChannel &ch);
  void WriteArrayImage(const IArrayImage &im);
  void WriteResourceDesc(const IResource &res);
  void WriteStr(const string &str);

  const IDesign *design_;
  ostream &os_;
};

}  // namespace iroha

#endif  // _writer_exp_writer_h_
