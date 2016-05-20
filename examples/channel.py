import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab_w = ITable(mod)
st10 = IState(tab_w)
tab_w.initialSt = st10
writer = design_tool.CreateChannelWrite(tab_w)
insn_w = IInsn(writer)
val_w = IRegister(tab_w, "w_reg")
val_w.SetInitialValue(123)
insn_w.inputs.append(val_w)
st10.insns.append(insn_w)
tab_w.states.append(st10)

tab_r = ITable(mod)
st20 = IState(tab_r)
tab_r.initialSt = st20
reader = design_tool.CreateChannelRead(tab_r)
insn_r = IInsn(reader)
val_r = IRegister(tab_r, "r_reg")
insn_r.outputs.append(val_r)
st20.insns.append(insn_r)
tab_r.states.append(st20)

ch = IChannel(d)
ch.SetReader(reader)
ch.SetWriter(writer)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
