import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

mi = ModuleImport(mod, "/tmp/a.iroha")
mod.module_import = mi

t_a = ModuleImportTap("a_in", None)
t_b = ModuleImportTap("b_in", None)
mi.taps.append(t_a)
mi.taps.append(t_b)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
