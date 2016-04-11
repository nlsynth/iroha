import sys
sys.path.append('../py')

from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab1 = ITable(mod)
st1 = IState(tab1)
tab1.states.append(st1)
tab1.initialSt = st1

tab2 = ITable(mod)
st2 = IState(tab2)
tab2.states.append(st2)
tab2.initialSt = st2


DesignTool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
