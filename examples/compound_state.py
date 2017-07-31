import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st0 = IState(tab)
tab.initialSt = st0
st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
tab.states.append(st0)
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
design_tool.AddNextState(st0, st1)
design_tool.AddNextState(st1, st2)
design_tool.AddNextState(st2, st3)

# task table.
task_tab = ITable(mod)
task_st0 = IState(task_tab)
task_tab.initialSt = task_st0
task_tab.states.append(task_st0)
task = design_tool.CreateTask(task_tab)
task_entry = IInsn(task)
task_st0.insns.append(task_entry)

# insns in main table.
caller = design_tool.CreateTaskCall(tab, task_tab)
call_insn = IInsn(caller)
st2.insns.append(call_insn)

counter = IRegister(tab, "counter")
counter.SetInitialValue(0)
adder = design_tool.GetBinOpResource(tab, "add", False, 32)
one = design_tool.AllocConstNum(tab, False, 32, 1)
add_insn = IInsn(adder)
add_insn.inputs.append(counter)
add_insn.inputs.append(one)
add_insn.outputs.append(counter)
st2.insns.append(add_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()

