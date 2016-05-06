import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

callee_tab = ITable(mod)
task = design_tool.CreateSiblingTask(callee_tab)
entry_insn = IInsn(task)
entry_insn.outputs.append(IRegister(callee_tab, "r_arg"))
st1 = IState(callee_tab)
st1.insns.append(entry_insn)
callee_tab.states.append(st1)
callee_tab.initialSt = st1

task.input_types.append(IValueType(False, 32))

print_res = design_tool.GetResource(callee_tab, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(design_tool.AllocConstNum(callee_tab, False, 32, 123))
st1.insns.append(print_insn)

caller_tab = ITable(mod)
call = design_tool.CreateSiblingTaskCall(caller_tab, callee_tab)
call.input_types.append(IValueType(False, 32))
call_insn = IInsn(call)
call_insn.inputs.append(design_tool.AllocConstNum(caller_tab, False, 32, 456))
st20 = IState(caller_tab)
st20.insns.append(call_insn)
caller_tab.states.append(st20)
caller_tab.initialSt = st20

st21 = IState(caller_tab)
caller_tab.states.append(st21)
wait_insn = IInsn(call)
wait_insn.operand = "wait"
st21.insns.append(wait_insn)

st22 = IState(caller_tab)
caller_tab.states.append(st22)

design_tool.AddNextState(st20, st21)
design_tool.AddNextState(st21, st22)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
