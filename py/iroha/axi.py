from .iroha import *
import design_tool

def CreateAxiPort(table, mem):
    res = design_tool.createResource(table, "axi-port")
    res.shared_reg = mem
    return res
