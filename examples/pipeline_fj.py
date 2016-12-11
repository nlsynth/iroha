# Fork-Join
#         /tab1\
# main_tab      last_tab
#         \tab2/
import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

tab1 = ITable(mod)
tab2 = ITable(mod)
last_tab = ITable(mod)
main_tab = ITable(mod)

def Call(caller, callee, task):
    # Entry insn.
    if not task:
        task = design_tool.CreateTask(callee)
        entry_insn = IInsn(task)
        task_st1 = IState(callee)
        task_st1.insns.append(entry_insn)
        callee.states.append(task_st1)
        callee.initialSt = task_st1
    # Calling insn.
    call = design_tool.CreateTaskCall(caller, callee)
    call_insn = IInsn(call)
    call_st = IState(caller)
    call_st.insns.append(call_insn)
    caller.states.append(call_st)
    #
    return task

Call(main_tab, tab1, None)
Call(main_tab, tab2, None)
task = Call(tab1, last_tab, None)
Call(tab2, last_tab, task)

# print
print_res = design_tool.GetResource(last_tab, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(design_tool.AllocConstNum(last_tab, False, 32, 123))
print_st = IState(last_tab)
print_st.insns.append(print_insn)
last_tab.states.append(print_st)
# connect states.
st_last = IState(main_tab)
main_tab.states.append(st_last)
main_tab.initialSt = main_tab.states[0]
design_tool.AddNextState(tab1.initialSt, tab1.states[1])
design_tool.AddNextState(tab2.initialSt, tab2.states[1])
design_tool.AddNextState(main_tab.initialSt, main_tab.states[1])
design_tool.AddNextState(main_tab.states[1], st_last)
design_tool.AddNextState(last_tab.initialSt, print_st)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
