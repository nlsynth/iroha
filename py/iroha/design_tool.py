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
    doValidateIds(design.channels)
    doValidateIds(design.modules)
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

def createResource(table, klass):
    design = table.module.design
    rc = design.findResourceClassByName(klass)
    res = IResource(table, rc)
    table.resources.append(res)
    return res

def CreateEmbedResource(table, name, fn, clk, rst):
    res = createResource(table, "embedded")
    res.resource_params.AddValue("EMBEDDED-MODULE", name)
    res.resource_params.AddValue("EMBEDDED-MODULE-FILE", fn)
    res.resource_params.AddValue("EMBEDDED-MODULE-CLOCK", clk)
    res.resource_params.AddValue("EMBEDDED-MODULE-RESET", rst)
    return res

def CreateIsolatedRegister(module, tab=None):
    if not tab:
        tab = ITable(module)
    return IRegister(tab, "r_%d" % len(tab.registers))

def CreateForeignRegister(table, foreign_reg):
    res = createResource(table, "foreign-reg")
    res.foreign_reg = foreign_reg
    return res

def CreateExtInput(table, name, width):
    res = createResource(table, "ext-input")
    res.resource_params.AddValue("INPUT", name)
    res.resource_params.AddValue("WIDTH", str(width))
    return res

def CreateExtOutput(table, name, width):
    res = createResource(table, "ext-output")
    res.resource_params.AddValue("OUTPUT", name)
    res.resource_params.AddValue("WIDTH", str(width))
    return res

def CreateSharedReg(table, name, width):
    res = createResource(table, "shared-reg")
    res.resource_params.AddValue("OUTPUT", name)
    res.resource_params.AddValue("WIDTH", str(width))
    return res

def CreateSharedRegReader(table, source):
    res = createResource(table, "shared-reg-reader")
    res.shared_reg = source
    return res

def CreateSharedRegWriter(table, source):
    res = createResource(table, "shared-reg-writer")
    res.shared_reg = source
    return res

def CreateSharedMemory(table, addr_width, data_width):
    res = createResource(table, "shared-memory")
    data_type = IValueType(False, data_width)
    res.array = IArray(addr_width, data_type, False, True)
    return res

def CreateSharedMemoryReader(table, source):
    res = createResource(table, "shared-memory-reader")
    res.shared_reg = source
    return res

def CreateSharedMemoryWriter(table, source):
    res = createResource(table, "shared-memory-writer")
    res.shared_reg = source
    return res

def CreateTask(table):
    return createResource(table, "task")

def CreateTaskCall(table, callee):
    res = createResource(table, "task-call")
    res.callee_table = callee
    return res

def CreateChannelRead(table):
    return createResource(table, "channel-read")

def CreateChannelWrite(table):
    return createResource(table, "channel-write")

def AddArgsToEmbeddedResource(res, arg):
    res.resource_params.AddValue("EMBEDDED-MODULE-ARGS", arg)

def SetHandShakeToEmbeddedResource(res, req, ack):
    res.resource_params.AddValue("EMBEDDED-MODULE-REQ", req)
    res.resource_params.AddValue("EMBEDDED-MODULE-ACK", ack)
