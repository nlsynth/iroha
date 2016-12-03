import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "M_top")
tab_top = ITable(mod_top)

mod_sub = IModule(d, "M_sub")
mod_sub.parent_module = mod_top
tab_sub = ITable(mod_sub)

# task tab
task_tab = ITable(mod_sub)
task = design_tool.CreateTask(task_tab)
task_st1 = IState(task_tab)
task_st2 = IState(task_tab)
task_tab.states.append(task_st1)
task_tab.states.append(task_st2)
task_tab.initialSt = task_st1
design_tool.AddNextState(task_st1, task_st2)
task_entry = IInsn(task)
task_st1.insns.append(task_entry)

r123 = design_tool.AllocConstNum(task_tab, False, 32, 123)
print_res = design_tool.GetResource(task_tab, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(r123)
task_st1.insns.append(print_insn)

# call from top
top_caller = design_tool.CreateTaskCall(tab_top, task_tab)
top_st1 = IState(tab_top)
top_st2 = IState(tab_top)
tab_top.initialSt = top_st1
tab_top.states.append(top_st1)
tab_top.states.append(top_st2)
top_call_insn = IInsn(top_caller)
top_st1.insns.append(top_call_insn)
design_tool.AddNextState(top_st1, top_st2)

# call from sibling (sub)
sub_caller = design_tool.CreateTaskCall(tab_sub, task_tab)
sub_st1 = IState(tab_sub)
sub_st2 = IState(tab_sub)
tab_sub.initialSt = sub_st1
tab_sub.states.append(sub_st1)
tab_sub.states.append(sub_st2)
sub_call_insn = IInsn(sub_caller)
sub_st1.insns.append(sub_call_insn)
design_tool.AddNextState(sub_st1, sub_st2)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
