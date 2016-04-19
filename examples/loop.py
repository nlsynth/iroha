import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)

st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
st4 = IState(tab)
st5 = IState(tab)
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
tab.states.append(st4)
tab.states.append(st5)

tab.initialSt = st1

counter = IRegister(tab, "counter")
counter.SetInitialValue(0)

ten = design_tool.AllocConstNum(tab, 32, 10)
gt = design_tool.GetBinOpResource(tab, "gt", 32)
cond = IRegister(tab, "cond")
cond.valueType = IValueType(0)

compare_insn = IInsn(gt)
compare_insn.inputs.append(counter)
compare_insn.inputs.append(ten)
compare_insn.outputs.append(cond)
st1.insns.append(compare_insn)
design_tool.AddNextState(st1, st2)
tr_insn = design_tool.AddNextState(st2, st3)
design_tool.AddNextState(st2, st5)

tr_insn.inputs.append(cond)
design_tool.AddNextState(st4, st1)

adder = design_tool.GetBinOpResource(tab, "add", 32)
one = design_tool.AllocConstNum(tab, 32, 1)
add_insn = IInsn(adder)
add_insn.inputs.append(counter)
add_insn.inputs.append(one)
add_insn.outputs.append(counter)
st3.insns.append(add_insn)
design_tool.AddNextState(st3, st4)

design_tool.ValidateIds(d)
w = DesignWriter(d)
w.Write()
