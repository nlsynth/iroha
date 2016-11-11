import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod = IModule(d, "mod")

mi = ModuleImport(mod, "/tmp/a.iroha")
mod.module_import = mi

design_tool.ValidateIds(d)
DesignWriter(d).Write()
