import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab = ITable(mod)

st0 = IState(tab)
st1 = IState(tab)
st2 = IState(tab)
tab.initialSt = st0
tab.states.append(st0)
tab.states.append(st1)
tab.states.append(st2)

design_tool.AddNextState(st0, st1)
design_tool.AddNextState(st1, st2)

df_res = design_tool.GetResource(tab, "dataflow-in")
df_insn = IInsn(df_res)
st0.insns.append(df_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
