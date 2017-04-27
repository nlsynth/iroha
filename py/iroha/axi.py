from .iroha import *
import design_tool

def CreateAxiMasterPort(table, mem):
    res = design_tool.createResource(table, "axi-master-port")
    res.parent_resource = mem
    return res
