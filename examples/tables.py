import sys
sys.path.append('../py')

from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

shared_reg = DesignTool.CreateSharedRegister(mod)

tab1 = ITable(mod)
st1 = IState(tab1)
tab1.states.append(st1)
tab1.initialSt = st1

foreign_reg1 = DesignTool.CreateForeignRegister(tab1, shared_reg)
assign = DesignTool.GetResource(tab1, "set")
# write
w_insn1 = IInsn(foreign_reg1)
r11 = IRegister(tab1, "r11")
r11.SetInitialValue(123)
w_insn1.inputs.append(r11)
st1.insns.append(w_insn1)

##
# Table 2
tab2 = ITable(mod)
st2 = IState(tab2)
tab2.states.append(st2)
tab2.initialSt = st2

foreign_reg2 = DesignTool.CreateForeignRegister(tab2, shared_reg)
# write
w_insn2 = IInsn(foreign_reg2)
r21 = IRegister(tab2, "r21")
r21.SetInitialValue(456)
w_insn2.inputs.append(r21)
st2.insns.append(w_insn2)

DesignTool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
