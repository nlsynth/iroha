import sys
sys.path.append('../py')

# python ext_mem.py | ../iroha - -v -opt array_to_mem

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

array = design_tool.CreateArrayResource(
    tab, 10, 32,
    True, # is_external
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
read_insn.outputs.append(read_data)
st2.insns.append(read_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()

