import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "mod_top")

mod_sub = IModule(d, "mod_sub")
mod_sub.parent_module = mod_top
tab = ITable(mod_sub)
st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
tab.initialSt = st1
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
design_tool.AddNextState(st1, st2)
design_tool.AddNextState(st2, st3)

ext_input = design_tool.CreateExtInput(tab, "data_in", 32)
iinsn = IInsn(ext_input)
r = IRegister(tab, "r")
iinsn.outputs.append(r)
st1.insns.append(iinsn)

ext_output = design_tool.CreateExtOutput(tab, "data_out", 32)
oinsn = IInsn(ext_output)
rc = design_tool.AllocConstNum(tab, False, 32, 123)
oinsn.inputs.append(rc)
st2.insns.append(oinsn)

ext_output_pulse = design_tool.CreateExtOutput(tab, "data_out_pulse", 1)
ext_output_pulse.resource_params.AddValue("DEFAULT-VALUE", "0")
oinsn_pulse = IInsn(ext_output_pulse)
rc = design_tool.AllocConstNum(tab, False, 32, 123)
oinsn_pulse.inputs.append(rc)
st2.insns.append(oinsn_pulse)

design_tool.ValidateIds(d)
w = DesignWriter(d)
w.Write()
