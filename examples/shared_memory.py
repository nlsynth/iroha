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

mem = design_tool.CreateSharedMemory(tab, 4, 32)

self_winsn = IInsn(mem)
addr = design_tool.AllocConstNum(tab, False, 4, 1)
wdata = design_tool.AllocConstNum(tab, False, 32, 123)
self_winsn.inputs.append(addr)
self_winsn.inputs.append(wdata)
st1.insns.append(self_winsn)

sib_tab = ITable(mod)
sib_st1 = IState(sib_tab)
sib_st2 = IState(sib_tab)
sib_st3 = IState(sib_tab)
sib_tab.initialSt = sib_st1
sib_tab.states.append(sib_st1)
sib_tab.states.append(sib_st2)
sib_tab.states.append(sib_st3)
design_tool.AddNextState(sib_st1, sib_st2)
design_tool.AddNextState(sib_st2, sib_st3)

sib_writer = design_tool.CreateSharedMemoryWriter(sib_tab, mem)
sib_winsn = IInsn(sib_writer)
sib_addr = design_tool.AllocConstNum(sib_tab, False, 4, 1)
sib_wdata = design_tool.AllocConstNum(sib_tab, False, 32, 456)
sib_winsn.inputs.append(sib_addr)
sib_winsn.inputs.append(sib_wdata)
sib_st1.insns.append(self_winsn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
