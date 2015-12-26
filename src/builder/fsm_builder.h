// -*- C++ -*-
#ifndef _builder_fsm_builder_h_
#define _builder_fsm_builder_h_

#include "iroha/common.h"

#include <map>

namespace iroha {

class Exp;
class ExpBuilder;

class FsmBuilder {
public:
  FsmBuilder(ITable *table, ExpBuilder *builder);

  void AddState(Exp *e);
  void SetInitialState(Exp *e);
  void ResolveInsns();

private:
  void Init();
  IInsn *BuildInsn(Exp *e);
  void BuildInsnParams(Exp *e, vector<IRegister *> *regs);

  ITable *table_;
  ExpBuilder *builder_;
  int initial_state_id_;

  map<int, IResource *> resources_;
  map<int, IRegister *> registers_;
  map<int, IState *> states_;
  map<int, Exp *> exps_;
};

}  // namespace iroha

#endif  // _builder_fsm_builder_h_
