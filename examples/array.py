import sys
sys.path.append('../py')

from iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st1 = IState(tab)
tab.initialSt = st1

array = DesignTool.CreateArrayResource(
    tab, 10, 32,
    False, # is_external
    True # is_ram
)

insn = IInsn(array)
addr = DesignTool.AllocConstNum(tab, 10, 0)
wdata = DesignTool.AllocConstNum(tab, 32, 123)
insn.inputs.append(addr)
insn.inputs.append(wdata)
st1.insns.append(insn)

DesignTool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
