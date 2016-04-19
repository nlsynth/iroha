import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st1 = IState(tab)

tab.states.append(st1)
tab.initialSt = st1

res = design_tool.CreateEmbedResource(tab, "mod_hello", "mod_hello.v")
insn = IInsn(res)
st1.insns.append(insn)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()

