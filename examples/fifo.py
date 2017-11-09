import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "mod_top")
mod_sub0 = IModule(d, "mod_sub0")
mod_sub0.parent_module = mod_top
mod_sub1 = IModule(d, "mod_sub1")
mod_sub1.parent_module = mod_top
tab0 = ITable(mod_sub0)
tab1 = ITable(mod_sub1)
tab_top = ITable(mod_top)

def PopulateStates(tab):
    st0 = IState(tab)
    st1 = IState(tab)
    st2 = IState(tab)
    tab.initialSt = st0
    tab.states.append(st0)
    tab.states.append(st1)
    tab.states.append(st2)
    design_tool.AddNextState(st0, st1)
    design_tool.AddNextState(st1, st2)

# tab0
PopulateStates(tab0)
st0 = tab0.states[0]
st1 = tab0.states[1]

fifo = design_tool.CreateFifo(tab0, 32, 2)
fifo_r = design_tool.CreateFifoReader(tab0, fifo)
fifo_w = design_tool.CreateFifoWriter(tab0, fifo)

w_insn = IInsn(fifo_w)
r_insn = IInsn(fifo_r)

ten = design_tool.AllocConstNum(tab0, False, 32, 10)
w_insn.inputs.append(ten)

r = IRegister(tab0, "r")
r_insn.outputs.append(r)

st0.insns.append(w_insn)
st1.insns.append(r_insn)

# tab1
PopulateStates(tab1)
st0 = tab1.states[0]
st1 = tab1.states[1]
fifo_r = design_tool.CreateFifoReader(tab1, fifo)
r_insn = IInsn(fifo_r)
r = IRegister(tab1, "r")
r_insn.outputs.append(r)
st0.insns.append(r_insn)

# tab_top
PopulateStates(tab_top)
st0 = tab_top.states[0]
st1 = tab_top.states[1]
fifo_w = design_tool.CreateFifoWriter(tab_top, fifo)
w_insn = IInsn(fifo_w)
ten = design_tool.AllocConstNum(tab_top, False, 32, 10)
w_insn.inputs.append(ten)
st0.insns.append(w_insn)

#
design_tool.ValidateIds(d)
DesignWriter(d).Write()
