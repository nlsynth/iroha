from .iroha import *
import design_tool

def CreateAxiPort(table, mem):
    res = design_tool.createResource(table, "axi-port")
    res.parent_resource = mem
    return res
