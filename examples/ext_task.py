import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

#mod_top = IModule(d, "M_top")
#mod.parent_module = mod_top

# task tab
tab = ITable(mod)
task = design_tool.CreateExtTask(tab, "fn")
task.output_types.append(IValueType(False, 32))
task.output_types.append(IValueType(False, 32))
task_done = design_tool.CreateExtTaskDone(tab, task)
task_done.input_types.append(IValueType(False, 32))
st1 = IState(tab)
st2 = IState(tab)
tab.states.append(st1)
tab.states.append(st2)
design_tool.AddNextState(st1, st2)
tab.initialSt = st1
task_entry = IInsn(task)
arg0 = IRegister(tab, "arg0")
task_entry.outputs.append(arg0)
arg1 = IRegister(tab, "arg1")
task_entry.outputs.append(arg1)
st1.insns.append(task_entry)
task_done_insn = IInsn(task_done)
st2.insns.append(task_done_insn)

res0 = IRegister(tab, "res0")
task_done_insn.inputs.append(res0)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
