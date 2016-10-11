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

reg_top = design_tool.CreateIsolatedRegister(mod_top, None)
foreign_reg1 = design_tool.CreateForeignRegister(tab_sub, reg_top)

reg_sub = design_tool.CreateIsolatedRegister(mod_sub, None)
foreign_reg2 = design_tool.CreateForeignRegister(tab_top, reg_sub)

design_tool.ValidateIds(d)
DesignWriter(d).Write()

