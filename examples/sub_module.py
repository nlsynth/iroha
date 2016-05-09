import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "M_top")
tab_top = ITable(mod_top)

mod_sub = IModule(d, "M_sub")
mod_sub.parent_module = mod_top
tab_sub = ITable(mod_sub)

# Callee.
callee_task = design_tool.CreateSubModuleTask(tab_sub)
entry_insn = IInsn(callee_task)
st10 = IState(tab_sub)
st10.insns.append(entry_insn)
tab_sub.states.append(st10)
tab_sub.initialSt = st10

# Caller.
call = design_tool.CreateSubModuleTaskCall(tab_top, tab_sub)
call_insn = IInsn(call)
st20 = IState(tab_top)
st20.insns.append(call_insn)
tab_top.states.append(st20)
tab_top.initialSt = st20


design_tool.ValidateIds(d)
DesignWriter(d).Write()
