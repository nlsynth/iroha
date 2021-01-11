// -*- C++ -*-
//
// Methods to manipulate design.
// (on the other hand design_util doesn't mutate)
//
#ifndef _design_design_tool_h_
#define _design_design_tool_h_

#include "iroha/i_design.h"

namespace iroha {

class DesignTool {
 public:
  static void Validate(IDesign *design);
  static void ValidateTable(ITable *table);
  static IInsn *AddNextState(IState *cur, IState *next);
  static IState *InsertNextState(IState *st);
  static void EraseInsn(IState *st, IInsn *insn);
  static void MoveInsn(IInsn *insn, IState *src_st, IState *dst_st);
  static IResource *GetOneResource(ITable *table, const string &class_name);
  static IResource *GetBinOpResource(ITable *table, const string &class_name,
                                     int width);
  static IResource *CreateShifterResource(ITable *table);
  static IResource *CreateArrayResource(ITable *table, int addres_width,
                                        int data_width, bool is_external,
                                        bool is_ram);
  static IResource *CreateExtTaskCallResource(ITable *table,
                                              const string &mod_name,
                                              const string &fn);
  static IResource *CreateExtCombinationalResource(ITable *table,
                                                   const string &mod_name,
                                                   const string &fn);
  static IResource *CreateTaskResource(ITable *table);
  static IResource *CreateTaskCallResource(ITable *caller, ITable *callee);
  static IResource *CreateDataFlowInResource(ITable *table);
  static IResource *CreateSharedRegResource(ITable *table, int width);
  static IResource *CreateSharedRegReaderResource(ITable *table,
                                                  IResource *reg);
  static IResource *CreateSharedRegWriterResource(ITable *table,
                                                  IResource *reg);
  static IResource *CreateSharedRegExtWriterResource(ITable *table,
                                                     IResource *reg);
  static IResource *CreateFifoResource(ITable *table, int width);
  static IResource *CreateFifoReaderResource(ITable *table, IResource *fifo);
  static IResource *CreateFifoWriterResource(ITable *table, IResource *fifo);
  static IResource *CopySimpleResource(IResource *res);
  static IRegister *AllocRegister(ITable *table, const string &name, int width);
  static IRegister *AllocConstNum(ITable *table, int width, uint64_t value);
  static void SetRegisterInitialValue(uint64_t value, IRegister *reg);
  static IInsn *CreateShiftInsn(IRegister *reg, bool to_left, int amount);
  static void DeleteInsn(IState *st, IInsn *insn);
};

}  // namespace iroha

#endif  // _design_design_tool_h_
