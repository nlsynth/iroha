import sys

class IDesign(object):
    def __init__(self):
        self.modules = []
        self.resource_classes = []
        self.installResourceClasses()

    def Write(self, writer):
        for m in self.modules:
            m.Write(writer)

    def installResourceClasses(self):
        self.resource_classes.append(IResourceClass("set"))
        self.resource_classes.append(IResourceClass("tr"))

    def findResourceClassByName(self, name):
        for rc in self.resource_classes:
            if rc.name == name:
                return rc
        return None

class IModule(object):
    def __init__(self, design, name):
        design.modules.append(self)
        self.design = design
        self.name = name
        self.tables = []

    def Write(self, writer):
        writer.ofh.write("(MODULE " + self.name + "\n")
        for t in self.tables:
            t.Write(writer)
        writer.ofh.write(")\n")

class ITable(object):
    def __init__(self, mod):
        mod.tables.append(self)
        self.module = mod
        self.resources = []
        self.states = []
        self.registers = []
        self.id = -1
        self.initialSt = None

    def Write(self, writer):
        writer.ofh.write(" (TABLE " + str(self.id) + "\n")
        writer.ofh.write("  (REGISTERS\n")
        for reg in self.registers:
            reg.Write(writer)
        writer.ofh.write("  )\n")
        writer.ofh.write("  (RESOURCES\n")
        for res in self.resources:
            res.Write(writer)
        writer.ofh.write("  )\n")
        if self.initialSt:
            writer.ofh.write("   (INITIAL " + str(self.id) + ")\n")
        for st in self.states:
            st.Write(writer)
        writer.ofh.write(" )")

class IState(object):
    def __init__(self, tab):
        tab.states.append(self)
        self.insns = []
        self.id = -1

    def Write(self, writer):
        writer.ofh.write("  (STATE " + str(self.id) + "\n")
        for insn in self.insns:
            insn.Write(writer)
        writer.ofh.write("  )\n")

class IInsn(object):
    def __init__(self, resource):
        self.id = -1
        self.inputs = []
        self.outputs = []
        self.target_states = []
        self.resource = resource

    def Write(self, writer):
        writer.ofh.write("   (INSN " + str(self.id) + " ")
        writer.ofh.write(self.resource.resource_class.name + " ")
        writer.ofh.write(str(self.resource.id) + " () ")
        # transitions, inputs, outputs
        self.writeStateIds(writer, self.target_states)
        writer.ofh.write(" ")
        self.writeRegisterIds(writer, self.inputs)
        writer.ofh.write(" ")
        self.writeRegisterIds(writer, self.outputs)
        writer.ofh.write(")\n")

    def writeStateIds(self, writer, sts):
        writer.ofh.write("()")

    def writeRegisterIds(self, writer, regs):
        writer.ofh.write("(")
        writer.ofh.write(" ".join([str(r.id) for r in regs]))
        writer.ofh.write(")")

class IRegister(object):
    def __init__(self, table, name):
        table.registers.append(self)
        self.id = -1
        self.name = name

    def Write(self, writer):
        writer.ofh.write("    (REGISTER " + str(self.id) + " ")
        if self.name:
            writer.ofh.write(self.name)
        else:
            writer.ofh.write("()")
        writer.ofh.write("\n")
        writer.ofh.write("     REG 32 ()\n")
        writer.ofh.write("    )\n")

class IResource(object):
    def __init__(self, table, resource_class):
        self.table = table
        self.resource_class = resource_class
        self.id = -1

    def Write(self, writer):
        writer.ofh.write("   (RESOURCE " + str(self.id))
        writer.ofh.write(" " + self.resource_class.name + "\n")
        # input, output, params
        writer.ofh.write("    () ()\n")
        writer.ofh.write("    ()\n")
        writer.ofh.write("   )\n")

class IResourceClass(object):
    def __init__(self, name):
        self.name = name

# IResourceClass
# IValueType
# IArray
# IChannel
# IValue
    
class DesignWriter(object):
    def __init__(self, design):
        self.design = design
        self.ofh = sys.stdout

    def Write(self):
        self.design.Write(self)

class DesignTool(object):
    @classmethod
    def GetResource(cls, table, name):
        design = table.module.design
        rc = design.findResourceClassByName(name)
        res = IResource(table, rc)
        table.resources.append(res)
        return res

    @classmethod
    def ValidateIds(cls, design):
        for mod in design.modules:
            cls.doValidateIds(mod.tables)
            for tab in mod.tables:
                cls.doValidateIds(tab.resources)
                cls.doValidateIds(tab.registers)
                cls.doValidateIds(tab.states)
                for st in tab.states:
                    cls.doValidateIds(st.insns)

    @classmethod
    def doValidateIds(cls, objs):
        usedIds = set()
        for obj in objs:
            if obj.id != -1:
                usedIds.add(obj.id)
        unusedId = 0
        for obj in objs:
            if obj.id == -1:
                next
            while unusedId in usedIds:
                unusedId = unusedId + 1
            obj.id = unusedId
