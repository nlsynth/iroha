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
design_tool.AddNextState(st1, st2)

r1 = IRegister(tab, "r1")
r1.SetInitialValue(1)
r1.SetType(False, 0)
r123 = IRegister(tab, "r123")
r123.SetInitialValue(123)

design_tool.ValidateIds(d)

print_res = design_tool.GetResource(tab, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(r123)
st1.insns.append(print_insn)

assert_res = design_tool.GetResource(tab, "assert")
assert_insn = IInsn(assert_res)
assert_insn.inputs.append(r1)
st1.insns.append(assert_insn)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
