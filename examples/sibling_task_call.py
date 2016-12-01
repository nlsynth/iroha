import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

# Callee.
callee_tab = ITable(mod)
task = design_tool.CreateSiblingTask(callee_tab)
entry_insn = IInsn(task)
arg_reg = IRegister(callee_tab, "r_arg")
entry_insn.outputs.append(arg_reg)
st10 = IState(callee_tab)
st10.insns.append(entry_insn)
callee_tab.states.append(st10)
callee_tab.initialSt = st10

task.input_types.append(IValueType(False, 32))

st11 = IState(callee_tab)
callee_tab.states.append(st11)
print_res = design_tool.GetResource(callee_tab, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(arg_reg)
st11.insns.append(print_insn)

design_tool.AddNextState(st10, st11)
st12 = IState(callee_tab)
callee_tab.states.append(st12)
design_tool.AddNextState(st11, st12)

# Caller.
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
DesignWriter(d).Write()
