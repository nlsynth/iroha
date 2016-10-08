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
tab.initialSt = st1
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
design_tool.AddNextState(st1, st2)
design_tool.AddNextState(st2, st3)

port_output = design_tool.CreatePortOutput(tab, "o", 32)
oinsn = IInsn(port_output)
rc = design_tool.AllocConstNum(tab, False, 32, 123)
oinsn.inputs.append(rc)
st1.insns.append(oinsn)

port_output_pulse = design_tool.CreatePortOutput(tab, "o_pulse", 32)
port_output_pulse.resource_params.AddValue("DEFAULT-VALUE", "0")
opinsn = IInsn(port_output_pulse)
rc = design_tool.AllocConstNum(tab, False, 32, 456)
opinsn.inputs.append(rc)
st1.insns.append(opinsn)

port_input = design_tool.CreatePortInput(tab, port_output)
iinsn = IInsn(port_input)
r = IRegister(tab, "r")
iinsn.outputs.append(r)
st2.insns.append(iinsn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
