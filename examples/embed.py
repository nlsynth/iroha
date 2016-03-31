import sys
sys.path.append('../py')

from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")
tab = ITable(mod)
st1 = IState(tab)

tab.states.append(st1)
tab.initialSt = st1

res = DesignTool.CreateEmbedResource(tab, "mod_hello", "mod_hello.v")
insn = IInsn(res)
st1.insns.append(insn)

DesignTool.ValidateIds(d)

w = DesignWriter(d)
w.Write()

