import sys
sys.path.append('../py')

from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

callee_tab = ITable(mod)
task = DesignTool.CreateSiblingTask(callee_tab)
entry_insn = IInsn(task)
st1 = IState(callee_tab)
st1.insns.append(entry_insn)
callee_tab.states.append(st1)
callee_tab.initialSt = st1

caller_tab = ITable(mod)
call = DesignTool.CreateSiblingTaskCall(caller_tab, callee_tab)
call_insn = IInsn(call)
st2 = IState(caller_tab)
st2.insns.append(call_insn)
caller_tab.states.append(st2)
caller_tab.initialSt = st2

DesignTool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
