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

mod_sibling = IModule(d, "M_sibling")
mod_sibling.parent_module = mod_top
tab_sibling = ITable(mod_sibling)

# task tab
task_tab = ITable(mod_sub)
task = design_tool.CreateTask(task_tab)
task.output_types.append(IValueType(False, 32))
task_st1 = IState(task_tab)
task_st2 = IState(task_tab)
task_tab.states.append(task_st1)
task_tab.states.append(task_st2)
task_tab.initialSt = task_st1
design_tool.AddNextState(task_st1, task_st2)
task_entry = IInsn(task)
task_st1.insns.append(task_entry)
callee_arg = IRegister(task_tab, "arg")
task_entry.outputs.append(callee_arg)

print_res = design_tool.GetResource(task_tab, "print")
print_insn = IInsn(print_res)
print_insn.inputs.append(callee_arg)
task_st2.insns.append(print_insn)

# call from top
top_caller = design_tool.CreateTaskCall(tab_top, task_tab)
top_caller.input_types.append(IValueType(False, 32))
top_st1 = IState(tab_top)
top_st2 = IState(tab_top)
tab_top.initialSt = top_st1
tab_top.states.append(top_st1)
tab_top.states.append(top_st2)
top_call_insn = IInsn(top_caller)
top_arg = design_tool.AllocConstNum(tab_top, False, 32, 10)
top_call_insn.inputs.append(top_arg)
top_st1.insns.append(top_call_insn)
design_tool.AddNextState(top_st1, top_st2)

# call from sibling table (sub)
sub_caller = design_tool.CreateTaskCall(tab_sub, task_tab)
sub_caller.input_types.append(IValueType(False, 32))
sub_st1 = IState(tab_sub)
sub_st2 = IState(tab_sub)
tab_sub.initialSt = sub_st1
tab_sub.states.append(sub_st1)
tab_sub.states.append(sub_st2)
sub_call_insn = IInsn(sub_caller)
sub_arg = design_tool.AllocConstNum(tab_sub, False, 32, 12)
sub_call_insn.inputs.append(sub_arg)
sub_st1.insns.append(sub_call_insn)
design_tool.AddNextState(sub_st1, sub_st2)

# call from sibling module
sibling_caller = design_tool.CreateTaskCall(tab_sibling, task_tab)
sibling_caller.input_types.append(IValueType(False, 32))
sibling_st1 = IState(tab_sibling)
sibling_st2 = IState(tab_sibling)
tab_sibling.initialSt = sibling_st1
tab_sibling.states.append(sibling_st1)
tab_sibling.states.append(sibling_st2)
sibling_call_insn = IInsn(sibling_caller)
sibling_arg = design_tool.AllocConstNum(tab_sibling, False, 32, 11)
sibling_call_insn.inputs.append(sibling_arg)
sibling_st1.insns.append(sibling_call_insn)
design_tool.AddNextState(sibling_st1, sibling_st2)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
