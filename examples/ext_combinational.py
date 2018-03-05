import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()

m = IModule(d, "mod")

tab = ITable(m)
st1 = IState(tab)
st2 = IState(tab)

tab.initialSt = st1
tab.states.append(st1)
tab.states.append(st2)
design_tool.AddNextState(st1, st2)

ext_comb = design_tool.CreateExtCombinational(tab, "ez")
ext_comb.input_types.append(IValueType(False, 32))
ext_comb.output_types.append(IValueType(False, 32))

iinsn = IInsn(ext_comb)
rc = design_tool.AllocConstNum(tab, False, 32, 123)
iinsn.inputs.append(rc)
r = IRegister(tab, "r")
iinsn.outputs.append(r)

st1.insns.append(iinsn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
