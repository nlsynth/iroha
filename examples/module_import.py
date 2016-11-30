import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

def GenSub():
    d = IDesign()
    mod = IModule(d, "tmpmod")
    tab = ITable(mod)
    st1 = IState(tab)
    st2 = IState(tab)
    st3 = IState(tab)
    tab.initialSt = st1
    tab.states.append(st1)
    tab.states.append(st2)
    tab.states.append(st3)
    #
    ext_input = design_tool.CreateExtInput(tab, "data_in", 32)
    iinsn = IInsn(ext_input)
    r = IRegister(tab, "r")
    iinsn.outputs.append(r)
    st1.insns.append(iinsn)
    #
    ext_output = design_tool.CreateExtOutput(tab, "data_out", 32)
    oinsn = IInsn(ext_output)
    oinsn.inputs.append(r)
    st2.insns.append(oinsn)
    #
    design_tool.AddNextState(st1, st2)
    design_tool.AddNextState(st2, st3)
    design_tool.ValidateIds(d)
    DesignWriter(d, "/tmp/tmpmod.iroha").Write()

GenSub()

d = IDesign()
mod_top = IModule(d, "mod_top")

#
mod_sub0 = IModule(d, "mod_sub0")
mod_sub0.parent_module = mod_top

tag = "tag1"

mi0 = ModuleImport(mod_sub0, "/tmp/tmpmod.iroha")
mod_sub0.module_import = mi0

#t0_a = ModuleImportTap("data_in", None, None)
t0_b = ModuleImportTap("data_out", tag, TapDesc.Create("shared-reg"))
#mi0.taps.append(t0_a)
mi0.taps.append(t0_b)

#
mod_sub1 = IModule(d, "mod_sub1")
mod_sub1.parent_module = mod_top

mi1 = ModuleImport(mod_sub0, "/tmp/tmpmod.iroha")
mod_sub1.module_import = mi1

t1_a = ModuleImportTap("data_in", tag, TapDesc.Create("shared-reg-reader"))
#t1_b = ModuleImportTap("data_out", None, None)
mi1.taps.append(t1_a)
#mi1.taps.append(t1_b)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
