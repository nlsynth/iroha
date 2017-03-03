import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab1 = ITable(mod)
sreg = design_tool.CreateSharedReg(tab1, "o", 32)

wtab = ITable(mod)
w = design_tool.CreateSharedRegWriter(wtab, sreg)
wst1 = IState(wtab)
wst2 = IState(wtab)
wst3 = IState(wtab)
wtab.initialSt = wst1
wtab.states.append(wst1)
wtab.states.append(wst2)
wtab.states.append(wst3)
design_tool.AddNextState(wst1, wst2)
design_tool.AddNextState(wst2, wst3)

winsn = IInsn(w)
rc = design_tool.AllocConstNum(wtab, False, 32, 123)
winsn.inputs.append(rc)
winsn.operand = "notify"
wst1.insns.append(winsn)

spinsn = IInsn(w)
spinsn.inputs.append(rc)
spinsn.operand = "put_mailbox"
wst2.insns.append(spinsn)

rtab = ITable(mod)
r = design_tool.CreateSharedRegReader(rtab, sreg)
rst1 = IState(rtab)
rst2 = IState(rtab)
rst3 = IState(rtab)
rtab.initialSt = rst1
rtab.states.append(rst1)
rtab.states.append(rst2)
rtab.states.append(rst3)
design_tool.AddNextState(rst1, rst2)
design_tool.AddNextState(rst2, rst3)

rinsn = IInsn(r)
reg = IRegister(rtab, "r_local")
rinsn.outputs.append(reg)
rinsn.operand = "wait_notify"
rst1.insns.append(rinsn)

sginsn = IInsn(r)
sginsn.inputs.append(rc)
sginsn.operand = "get_mailbox"
rst2.insns.append(sginsn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
