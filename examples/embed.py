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

res = design_tool.CreateEmbeddedExtTaskCall(tab, "mod_hello", "mod_hello.v", "clk", "rst_n")

design_tool.AddEmbeddedModuleIO(res, ["x:32", "y:32"], ["z"])

insn = IInsn(res)
st1.insns.append(insn)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()

