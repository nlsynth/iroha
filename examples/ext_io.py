import sys
sys.path.append('../py')

from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
tab.initialSt = st1
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
DesignTool.AddNextState(st1, st2)
DesignTool.AddNextState(st2, st3)

ext_input = DesignTool.CreateExtInput(tab, "data_in", 32)
iinsn = IInsn(ext_input)
r = IRegister(tab, "r")
iinsn.outputs.append(r)
st1.insns.append(iinsn)

ext_output = DesignTool.CreateExtOutput(tab, "data_out", 32)
oinsn = IInsn(ext_output)
rc = DesignTool.AllocConstNum(tab, 32, 123)
oinsn.inputs.append(rc)
st2.insns.append(oinsn)

DesignTool.ValidateIds(d)
w = DesignWriter(d)
w.Write()
