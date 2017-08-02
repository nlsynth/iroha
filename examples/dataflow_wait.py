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
ext_input = design_tool.CreateExtInput(tab, "data_in", 0)
in_insn = IInsn(ext_input)
in_r = IRegister(tab, "r")
in_r.isWire = True
in_r.valueType = IValueType(False, 0)
in_insn.outputs.append(in_r)
tab.states[0].insns.append(in_insn)
df_in = design_tool.GetResource(tab, "dataflow-in")
df_insn = IInsn(df_in)
df_insn.inputs.append(in_r)
tab.states[0].insns.append(df_insn)
# task call
caller = design_tool.CreateTaskCall(tab, task_tab)
call_insn = IInsn(caller)
st2.insns.append(call_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
