import sys
sys.path.append('../py')

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

mem = design_tool.CreateSharedMemory(tab, 4, 32)

axi_slave_port = axi.CreateAxiSlavePort(tab, mem)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
