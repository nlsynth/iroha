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

ticker = design_tool.CreateTicker(tab)
insn = IInsn(ticker)
st2.insns.append(insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
