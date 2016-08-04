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

res = design_tool.CreateEmbedResource(tab, "mod_hello", "mod_hello.v", "clk", "rst_n")
design_tool.SetHandShakeToEmbeddedResource(res, "req_hello", "ack_hello")
design_tool.AddArgsToEmbeddedResource(res, "arg_hello")
res.input_types.append(IValueType(False, 32))


insn = IInsn(res)
data = design_tool.AllocConstNum(tab, False, 32, 123)
insn.inputs.append(data)

st1.insns.append(insn)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()

