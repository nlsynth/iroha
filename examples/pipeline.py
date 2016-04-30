import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab1 = ITable(mod)
tab2 = ITable(mod)
tab3 = ITable(mod)
main_tab = ITable(mod)

def Call(caller, callee):
    # Entry insn.
    task = design_tool.CreateSiblingTask(callee)
    entry_insn = IInsn(task)
    task_st1 = IState(callee)
    task_st1.insns.append(entry_insn)
    callee.states.append(task_st1)
    callee.initialSt = task_st1
    # Calling insn.
    call = design_tool.CreateSiblingTaskCall(caller, callee)
    call_insn = IInsn(call)
    call_st = IState(caller)
    call_st.insns.append(call_insn)
    caller.states.append(call_st)

Call(main_tab, tab1)
Call(tab1, tab2)
Call(tab2, tab3)

main_tab.initialSt = main_tab.states[0]
# print
print_res = design_tool.GetResource(tab3, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(design_tool.AllocConstNum(tab3, False, 32, 123))
print_st = IState(tab3)
print_st.insns.append(print_insn)
tab3.states.append(print_st)
# connect states.
design_tool.AddNextState(tab1.initialSt, tab1.states[1])
design_tool.AddNextState(tab2.initialSt, tab2.states[1])
design_tool.AddNextState(tab3.initialSt, print_st)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
