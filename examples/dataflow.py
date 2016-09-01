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
tab.initialSt = st0
tab.states.append(st0)
tab.states.append(st1)
tab.states.append(st2)

design_tool.AddNextState(st0, st1)
design_tool.AddNextState(st1, st2)

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

counter = IRegister(tab, "counter")
counter.SetInitialValue(0)
adder = design_tool.GetBinOpResource(tab, "add", False, 32)
one = design_tool.AllocConstNum(tab, False, 32, 1)
add_insn = IInsn(adder)
add_insn.inputs.append(counter)
add_insn.inputs.append(one)
add_insn.outputs.append(counter)
st1.insns.append(add_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
