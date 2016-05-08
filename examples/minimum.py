import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st1 = IState(tab)
st2 = IState(tab)

tab.states.append(st1)
tab.states.append(st2)
tab.initialSt = st1

r1 = IRegister(tab, "r1")
r2 = IRegister(tab, "r2")

r2.SetInitialValue(123)
r2.valueType = IValueType(True, 32)

assign = design_tool.GetResource(tab, "set")
insn = IInsn(assign)
insn.inputs.append(r2)
insn.outputs.append(r1)
st1.insns.append(insn)

tr = design_tool.GetResource(tab, "tr")
tr_insn = IInsn(tr)
st1.insns.append(tr_insn)
tr_insn.target_states.append(st2)

d.resource_params.AddValue("RESET-POLARITY", "true")
mod.resource_params.AddValue("RESET-NAME", "reset")


design_tool.ValidateIds(d)
DesignWriter(d).Write()
