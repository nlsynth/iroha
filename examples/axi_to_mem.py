import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()

mod_root = IModule(d, "mod_root")

mod = IModule(d, "mod")
mod.parent_module = mod_root
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

axi_master_port = axi.CreateAxiMasterPort(tab, mem)
axi_master_port.resource_params.AddValue("PORT-NAME-PREFIX", "a1_")
# 0 to use shared port. 1 to use dual port memory and exclusive mode.
# axi_master_port.resource_params.AddValue("SRAM-PORT-INDEX", "0")

rinsn = IInsn(axi_master_port)
addr = design_tool.AllocConstNum(tab, False, 32, 128)
rinsn.inputs.append(addr)
rinsn.operand = "read"
st1.insns.append(rinsn)

winsn = IInsn(axi_master_port)
addr = design_tool.AllocConstNum(tab, False, 32, 128)
winsn.inputs.append(addr)
winsn.operand = "write"
st2.insns.append(winsn)

minsn = IInsn(mem)
maddr = design_tool.AllocConstNum(tab, False, 4, 0)
mdata = IRegister(tab, "r1")
minsn.inputs.append(maddr)
minsn.outputs.append(mdata)
st3.insns.append(minsn)

design_tool.ValidateIds(d)
w = DesignWriter(d)
w.Write()
