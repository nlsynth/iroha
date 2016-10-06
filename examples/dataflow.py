import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab = ITable(mod)

st0 = IState(tab)
st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
st4 = IState(tab)
st5 = IState(tab)
tab.initialSt = st0
tab.states.append(st0)
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
tab.states.append(st4)
tab.states.append(st5)

design_tool.AddNextState(st0, st1)
design_tool.AddNextState(st1, st2)
design_tool.AddNextState(st3, st5)
design_tool.AddNextState(st4, st5)

ext_input = design_tool.CreateExtInput(tab, "data_in", 0)
in_insn = IInsn(ext_input)
in_r = IRegister(tab, "r")
in_r.isWire = True
in_insn.outputs.append(in_r)
st0.insns.append(in_insn)

df_res = design_tool.GetResource(tab, "dataflow-in")
df_insn = IInsn(df_res)
df_insn.inputs.append(in_r)
st0.insns.append(df_insn)

adder = design_tool.GetBinOpResource(tab, "add", False, 32)
counter = IRegister(tab, "counter")
one = design_tool.AllocConstNum(tab, False, 32, 1)
add_insn = IInsn(adder)
add_insn.inputs.append(counter)
add_insn.inputs.append(one)
add_insn.outputs.append(counter)
st1.insns.append(add_insn)

gt = design_tool.GetBinOpResource(tab, "gt", False, 32)
ten = design_tool.AllocConstNum(tab, False, 32, 10)
counter.SetInitialValue(0)
cond = IRegister(tab, "cond")
cond.valueType = IValueType(False, 0)
gt_insn = IInsn(gt)
gt_insn.inputs.append(counter)
gt_insn.inputs.append(one)
gt_insn.outputs.append(cond)
st1.insns.append(gt_insn)

tr_insn = design_tool.AddNextState(st2, st3)
tr_insn.inputs.append(cond)
design_tool.AddNextState(st2, st4)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
