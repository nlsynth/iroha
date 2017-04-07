#
# mtab
#  shared-reg <- shared-reg-writer
#        ^
# ptab   |
#  dataflow-in

import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

mtab = ITable(mod)
sreg = design_tool.CreateSharedReg(mtab, "mb", 32)
wreg = design_tool.CreateSharedRegWriter(mtab, sreg)

mst0 = IState(mtab)
mst1 = IState(mtab)
mtab.initialSt = mst0
mtab.states.append(mst0)
mtab.states.append(mst1)
design_tool.AddNextState(mst0, mst1)

winsn = IInsn(wreg)
winsn.operand = "notify"
rc = design_tool.AllocConstNum(mtab, False, 32, 123)
winsn.inputs.append(rc)
mst1.insns.append(winsn)


ptab = ITable(mod)

pst0 = IState(ptab)
pst1 = IState(ptab)
pst2 = IState(ptab)
ptab.initialSt = pst0
ptab.states.append(pst0)
ptab.states.append(pst1)
ptab.states.append(pst2)
design_tool.AddNextState(pst0, pst1)
design_tool.AddNextState(pst1, pst2)

df_res = design_tool.GetResource(ptab, "dataflow-in")
df_insn = IInsn(df_res)
df_arg = IRegister(ptab, "arg")
df_insn.outputs.append(df_arg)
ptab.states[0].insns.append(df_insn)

df_res.parent_resource = sreg

print_res = design_tool.GetResource(ptab, "print")
print_insn = IInsn(print_res)
r123 = IRegister(ptab, "r123")
r123.SetInitialValue(123)
print_insn.inputs.append(r123)
pst2.insns.append(print_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
