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

fifo = design_tool.CreateFifo(tab, 32, 2)
fifo_r = design_tool.CreateFifoReader(tab, fifo)
fifo_w = design_tool.CreateFifoWriter(tab, fifo)

w_insn = IInsn(fifo_w)
r_insn = IInsn(fifo_r)

ten = design_tool.AllocConstNum(tab, False, 32, 10)
w_insn.inputs.append(ten)

r = IRegister(tab, "r")
r_insn.outputs.append(r)

st0.insns.append(w_insn)
st1.insns.append(r_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
