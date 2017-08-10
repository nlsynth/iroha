import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "M_top")

tab = ITable(mod)
st1 = IState(tab)
st2 = IState(tab)
st3 = IState(tab)
tab.states.append(st1)
tab.states.append(st2)
tab.states.append(st3)
design_tool.AddNextState(st1, st2)
design_tool.AddNextState(st2, st3)
tab.initialSt = st1

flow_call = design_tool.CreateEmbeddedExtFlowCall(tab, "mod_flow", "mod_flow.v", "clk", "rst_n")
flow_wait = design_tool.CreateExtFlowResult(tab, flow_call)

flow_call.input_types.append(IValueType(False, 32))
flow_wait.output_types.append(IValueType(False, 32))

call_insn = IInsn(flow_call)
arg0 = IRegister(tab, "arg0")
call_insn.inputs.append(arg0)
wait_insn = IInsn(flow_wait)
ret0 = IRegister(tab, "ret0")
wait_insn.outputs.append(ret0)

st1.insns.append(call_insn)
st2.insns.append(wait_insn)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
