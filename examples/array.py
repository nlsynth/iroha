import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st1 = IState(tab)
st2 = IState(tab)
tab.initialSt = st1

tab.states.append(st1)
tab.states.append(st2)
design_tool.AddNextState(st1, st2)

array = design_tool.CreateArrayResource(
    tab, 10, 32,
    False, # is_external
    True # is_ram
)

write_insn = IInsn(array)
addr = design_tool.AllocConstNum(tab, False, 10, 0)
wdata = design_tool.AllocConstNum(tab, False, 32, 123)
write_insn.inputs.append(addr)
write_insn.inputs.append(wdata)
st1.insns.append(write_insn)

read_insn = IInsn(array)
read_insn.inputs.append(addr)
read_data = IRegister(tab, "r1")
read_insn.outputs.append(addr)
st2.insns.append(read_insn)
design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
