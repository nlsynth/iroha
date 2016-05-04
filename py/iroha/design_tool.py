from .iroha import *

def GetResource(table, name):
    design = table.module.design
    rc = design.findResourceClassByName(name)
    for res in table.resources:
        if res.resource_class == rc:
            return res
    res = IResource(table, rc)
    table.resources.append(res)
    return res

def GetBinOpResource(table, name, isSigned, width):
    design = table.module.design
    rc = design.findResourceClassByName(name)
    for res in table.resources:
        if res.resource_class == rc:
            if res.input_types[0].width == width:
                return res
    res = IResource(table, rc)
    t = IValueType(isSigned, width)
    res.input_types.append(t)
    res.input_types.append(t)
    if name == "gt":
        res.output_types.append(IValueType(isSigned, 0))
    else:
        res.output_types.append(t)
    table.resources.append(res)
    return res

def ValidateIds(design):
    for mod in design.modules:
        doValidateIds(mod.tables)
        for tab in mod.tables:
            insns = []
            doValidateIds(tab.resources)
            doValidateIds(tab.registers)
            doValidateIds(tab.states)
            for st in tab.states:
                for insn in st.insns:
                    insns.append(insn)
            doValidateIds(insns)

def doValidateIds(objs):
    usedIds = set()
    for obj in objs:
        if obj.id != -1:
            usedIds.add(obj.id)
    unusedId = 1
    for obj in objs:
        if obj.id != -1:
            continue
        while unusedId in usedIds:
            unusedId = unusedId + 1
        obj.id = unusedId
        unusedId = unusedId + 1

def CreateArrayResource(table, addr_width, data_width, is_external, is_ram):
    data_type = IValueType(False, data_width)
    res = GetResource(table, "array")
    res.array = IArray(addr_width, data_type, is_external, is_ram)
    return res

def AllocConstNum(table, isSigned, width, val):
    reg = IRegister(table, "")
    reg.initialValue = IValue(val)
    reg.valueType = IValueType(isSigned, width)
    reg.isConst = True
    return reg

def AddNextState(st1, st2):
    tr = GetResource(st1.tab, "tr")
    tr_insn = None
    for insn in st1.insns:
        if insn.resource == tr:
            tr_insn = insn
    if not tr_insn:
        tr_insn = IInsn(tr)
        st1.insns.append(tr_insn)
    tr_insn.target_states.append(st2)
    return tr_insn

def CreateEmbedResource(table, name, fn):
    design = table.module.design
    rc = design.findResourceClassByName("embedded")
    res = IResource(table, rc)
    res.resource_params.AddValue("EMBEDDED-MODULE", name)
    res.resource_params.AddValue("EMBEDDED-MODULE-FILE", fn)
    table.resources.append(res)
    return res

def CreateSharedRegister(module, tab=None):
    if not tab:
        tab = ITable(module)
    return IRegister(tab, "r")

def CreateForeignRegister(table, shared_reg):
    design = table.module.design
    rc = design.findResourceClassByName("foreign-reg")
    res = IResource(table, rc)
    res.foreign_reg = shared_reg
    table.resources.append(res)
    return res

def CreateExtInput(table, name, width):
    design = table.module.design
    rc = design.findResourceClassByName("ext_input")
    res = IResource(table, rc)
    res.resource_params.AddValue("INPUT", name)
    res.resource_params.AddValue("WIDTH", str(width))
    table.resources.append(res)
    return res

def CreateExtOutput(table, name, width):
    design = table.module.design
    rc = design.findResourceClassByName("ext_output")
    res = IResource(table, rc)
    res.resource_params.AddValue("OUTPUT", name)
    res.resource_params.AddValue("WIDTH", str(width))
    table.resources.append(res)
    return res

def CreateSiblingTask(table):
    design = table.module.design
    rc = design.findResourceClassByName("sibling-task")
    res = IResource(table, rc)
    table.resources.append(res)
    return res

def CreateSiblingTaskCall(table, callee):
    design = table.module.design
    rc = design.findResourceClassByName("sibling-task-call")
    res = IResource(table, rc)
    res.callee_table = callee
    table.resources.append(res)
    return res
