import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "M_top")
tab_top = ITable(mod_top)
st1_top = IState(tab_top)

mod_sub = IModule(d, "M_sub")
mod_sub.parent_module = mod_top
tab_sub = ITable(mod_sub)

design_tool.ValidateIds(d)

w = DesignWriter(d)
w.Write()
