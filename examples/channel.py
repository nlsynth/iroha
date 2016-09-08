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
writer.input_types.append(IValueType(False, 32))
insn_w = IInsn(writer)
val_w = IRegister(tab_w, "w_reg")
val_w.SetInitialValue(123)
insn_w.inputs.append(val_w)
st10.insns.append(insn_w)
tab_w.states.append(st10)
st11 = IState(tab_w)
tab_w.states.append(st11)
design_tool.AddNextState(st10, st11)

tab_r = ITable(mod)
st20 = IState(tab_r)
tab_r.initialSt = st20
reader = design_tool.CreateChannelRead(tab_r)
reader.output_types.append(IValueType(False, 32))
insn_r = IInsn(reader)
val_r = IRegister(tab_r, "r_reg")
insn_r.outputs.append(val_r)
st20.insns.append(insn_r)
tab_r.states.append(st20)
#
st21 = IState(tab_r)
tab_r.states.append(st21)
print_res = design_tool.GetResource(tab_r, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(val_r)
st21.insns.append(print_insn)
design_tool.AddNextState(st20, st21)
#
st22 = IState(tab_r)
tab_r.states.append(st22)
design_tool.AddNextState(st21, st22)

ch = IChannel(d)
ch.SetReader(reader)
ch.SetWriter(writer)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
