import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

def CreateTable(mod):
    tab = ITable(mod)
    st0 = IState(tab)
    st1 = IState(tab)
    tab.initialSt = st0
    design_tool.AddNextState(st0, st1)
    tab.states.append(st0)
    tab.states.append(st1)
    return tab

tab0 = CreateTable(mod)
tab1 = CreateTable(mod)

# Kicks tab0 by external input
ext_input = design_tool.CreateExtInput(tab0, "data_in", 0)
in_insn = IInsn(ext_input)
in_r = IRegister(tab0, "r")
in_r.isWire = True
in_r.valueType = IValueType(False, 0)
in_insn.outputs.append(in_r)
tab0.states[0].insns.append(in_insn)
df_in = design_tool.GetResource(tab0, "dataflow-in")
df_insn = IInsn(df_in)
df_insn.inputs.append(in_r)
tab0.states[0].insns.append(df_insn)

# Kicks tab1
sreg = design_tool.CreateSharedReg(tab0, "o", 0)
sreg.resource_params.AddValue("DEFAULT-VALUE", "0")
sinsn = IInsn(sreg)
bit0 = design_tool.AllocConstNum(tab0, False, 0, 1)
sinsn.inputs.append(bit0)
tab0.states[-1].insns.append(sinsn)

# Kicked by tab0
rreg = design_tool.CreateSharedRegReader(tab1, sreg)
rinsn = IInsn(rreg)
rwire = IRegister(tab1, "r")
rwire.isWire = True
rwire.valueType = IValueType(False, 0)
rinsn.outputs.append(rwire)
tab1.states[0].insns.append(rinsn)
df1_in = design_tool.GetResource(tab1, "dataflow-in")
df1_insn = IInsn(df1_in)
df1_insn.inputs.append(rwire)
tab1.states[0].insns.append(df1_insn)

# Triggers ext port
ext_output = design_tool.CreateExtOutput(tab1, "data_out", 0)
ext_output.resource_params.AddValue("DEFAULT-VALUE", "0")
oinsn = IInsn(ext_output)
bit1 = design_tool.AllocConstNum(tab1, False, 0, 1)
oinsn.inputs.append(bit1)
tab1.states[-1].insns.append(oinsn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
