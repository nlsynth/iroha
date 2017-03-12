import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "M_top")

# task tab
tab = ITable(mod)
task = design_tool.CreateExtTask(tab, "fn")
task_done = design_tool.CreateExtTaskDone(tab, task)
st1 = IState(tab)
st2 = IState(tab)
tab.states.append(st1)
tab.states.append(st2)
design_tool.AddNextState(st1, st2)
tab.initialSt = st1
task_entry = IInsn(task)
st1.insns.append(task_entry)
task_done_insn = IInsn(task_done)
st2.insns.append(task_done_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
