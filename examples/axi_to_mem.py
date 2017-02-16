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

tab.initialSt = st1
tab.states.append(st1)
tab.states.append(st2)

design_tool.AddNextState(st1, st2)

mem = design_tool.CreateSharedMemory(tab, 4, 32)

axi_port = axi.CreateAxiPort(tab, mem)

insn = IInsn(axi_port)
addr = design_tool.AllocConstNum(tab, False, 32, 1)
insn.inputs.append(addr)
insn.operand = "read"
st1.insns.append(insn)

minsn = IInsn(mem)
maddr = design_tool.AllocConstNum(tab, False, 4, 0)
mdata = IRegister(tab, "r1")
minsn.inputs.append(maddr)
minsn.outputs.append(mdata)
st2.insns.append(minsn)

design_tool.ValidateIds(d)
w = DesignWriter(d)
w.Write()
