import sys
sys.path.append('../py')

from iroha import *
from iroha.iroha import *

d = IDesign()
mod_top = IModule(d, "M_top")
tab_top = ITable(mod_top)

mod_sub_1 = IModule(d, "M_sub_1")
mod_sub_1.parent_module = mod_top
tab_sub_1 = ITable(mod_sub_1)

mod_sub_2 = IModule(d, "M_sub_2")
mod_sub_2.parent_module = mod_top
tab_sub_2 = ITable(mod_sub_2)

port_source_sub_1 = design_tool.CreatePortOutput(tab_sub_1, "o_1", 32)
port_source_top = design_tool.CreatePortOutput(tab_top, "o_t", 32)

# sub1 -> top
port_sink_top = design_tool.CreatePortInput(tab_top, port_source_sub_1)
# top -> sub2
port_sink_sub_1 = design_tool.CreatePortInput(tab_sub_2, port_source_top)
# sub1 -> sub2
port_sink_sub_2 = design_tool.CreatePortInput(tab_sub_2, port_source_sub_1)

design_tool.ValidateIds(d)
DesignWriter(d).Write()
