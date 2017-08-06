import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "M_top")

tab = ITable(mod)
st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
design_tool.AddNextState(st1, st2)
design_tool.AddNextState(st2, st3)
tab.initialSt = st1

# task_call = design_tool.CreateExtTaskCall(tab, "fn")
task_call = design_tool.CreateEmbeddedExtTaskCall(tab, "mod_task", "mod_task.v", "clk", "rst_n")
task_wait = design_tool.CreateExtTaskWait(tab, task_call)

task_call.input_types.append(IValueType(False, 32))
task_wait.output_types.append(IValueType(False, 32))

call_insn = IInsn(task_call)
arg0 = IRegister(tab, "arg0")
call_insn.inputs.append(arg0)
wait_insn = IInsn(task_wait)
ret0 = IRegister(tab, "ret0")
wait_insn.outputs.append(ret0)

st1.insns.append(call_insn)
st2.insns.append(wait_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
