import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "M_top")
tab_top = ITable(mod_top)

reg_top = design_tool.CreateSharedRegister(mod_top, None)

mod_sub = IModule(d, "M_sub")
mod_sub.parent_module = mod_top
tab_sub = ITable(mod_sub)

foreign_reg1 = design_tool.CreateForeignRegister(tab_sub, reg_top)

design_tool.ValidateIds(d)
DesignWriter(d).Write()

