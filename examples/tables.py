import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab1 = ITable(mod)
st11 = IState(tab1)
tab1.states.append(st11)
tab1.initialSt = st11
st12 = IState(tab1)
tab1.states.append(st12)
design_tool.AddNextState(st11, st12)
source = IRegister(tab1, "source")
assign = design_tool.GetResource(tab1, "set")
w_insn = IInsn(assign)
w_insn.inputs.append(design_tool.AllocConstNum(tab1, False, 32, 10))
w_insn.outputs.append(source)
st11.insns.append(w_insn)

tab2 = ITable(mod)
st21 = IState(tab2)
tab2.states.append(st21)
tab2.initialSt = st21
st22 = IState(tab2)
tab2.states.append(st22)
design_tool.AddNextState(st21, st22)
st23 = IState(tab2)
tab2.states.append(st23)
design_tool.AddNextState(st22, st23)

foreign_reg1 = design_tool.CreateForeignRegister(tab2, source)
# read
r_insn = IInsn(foreign_reg1)
r = IRegister(tab2, "r")
r_insn.outputs.append(r)
st22.insns.append(r_insn)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
