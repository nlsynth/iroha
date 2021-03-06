// -*- C++ -*-
#ifndef _opt_optimizer_log_h_
#define _opt_optimizer_log_h_

#include <map>
#include <set>
#include <sstream>

#include "iroha/common.h"

namespace iroha {
namespace opt {

class OptimizerLogSection;

// Per design object to store debug strings.
class OptimizerLog {
 public:
  OptimizerLog();
  ~OptimizerLog();

  // Called once at the start.
  void Enable();
  // Called at the end.
  void WriteToFiles(const string &baseFn);
  // Called at beginning of each pass.
  void StartPass(const string &name);
  // Dump into a separate file from the main output from the pass.
  void StartSubSection(const string &section, bool isHtml);
  void ClearSubSection();
  // Called by optimizers.
  bool IsEnabled();
  void DumpTable(const ITable *tab);
  ostream &GetDumpStream();
  ostream &Table(const ITable *tab);
  ostream &State(const IState *st);
  ostream &Insn(const IInsn *insn);
  ostream &Reg(const IRegister *reg);
  void SetStateColorIndex(const IState *st, int idx);

  // Called by HTML dumper.
  string GetTableAnnotation(const ITable *tab) const;
  string GetStateAnnotation(const IState *st) const;
  string GetInsnAnnotation(const IInsn *insn) const;
  string GetRegAnnotation(const IRegister *reg) const;
  int GetStateColorIndex(const IState *st) const;

 private:
  void UpdateFileName(bool isHtml);
  OptimizerLogSection *GetSection(const string &name);

  bool enabled_;
  int current_pass_index_;
  string current_pass_name_;
  string current_section_name_;
  string current_file_name_;
  map<string, OptimizerLogSection *> sections_;

  map<const ITable *, ostringstream> table_;
  map<const IState *, ostringstream> state_;
  map<const IInsn *, ostringstream> insn_;
  map<const IRegister *, ostringstream> reg_;
  map<const IState *, int> state_color_;
};

}  // namespace opt
}  // namespace iroha

#endif  // _opt_optimizer_log_h_
