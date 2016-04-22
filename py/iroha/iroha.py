"""Iroha IR builder in Python"""

import sys

class IDesign(object):
    def __init__(self):
        self.modules = []
        self.resource_classes = []
        self.installResourceClasses()
        self.resourceParams = ResourceParams()

    def Write(self, writer):
        self.resourceParams.Write(writer)
        for m in self.modules:
            m.Write(writer)

    def installResourceClasses(self):
        self.resource_classes.append(IResourceClass("set"))
        self.resource_classes.append(IResourceClass("tr"))
        self.resource_classes.append(IResourceClass("add"))
        self.resource_classes.append(IResourceClass("gt"))
        self.resource_classes.append(IResourceClass("array"))
        self.resource_classes.append(IResourceClass("embedded"))
        self.resource_classes.append(IResourceClass("assert"))
        self.resource_classes.append(IResourceClass("print"))
        self.resource_classes.append(IResourceClass("ext_input"))
        self.resource_classes.append(IResourceClass("ext_output"))
        self.resource_classes.append(IResourceClass("foreign-reg"))
        self.resource_classes.append(IResourceClass("sibling-task"))
        self.resource_classes.append(IResourceClass("sibling-task-call"))

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
        self.parent_module = None
        self.resource_params = ResourceParams()

    def Write(self, writer):
        writer.ofh.write("(MODULE " + self.name + "\n")
        writer.ofh.write(" ")
        self.resource_params.Write(writer)
        if self.parent_module:
            writer.ofh.write(" (PARENT " + self.parent_module.name + ")\n")
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
            writer.ofh.write("  (INITIAL " + str(self.initialSt.id) + ")\n")
        for st in self.states:
            st.Write(writer)
        writer.ofh.write(" )\n")

class IState(object):
    def __init__(self, tab):
        self.tab = tab
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
        self.writeIds(writer, self.target_states)
        writer.ofh.write(" ")
        self.writeIds(writer, self.inputs)
        writer.ofh.write(" ")
        self.writeIds(writer, self.outputs)
        writer.ofh.write(")\n")

    def writeIds(self, writer, objs):
        """objs can be list of either IState or IRegister."""
        writer.ofh.write("(")
        writer.ofh.write(" ".join([str(o.id) for o in objs]))
        writer.ofh.write(")")

class IRegister(object):
    def __init__(self, table, name):
        table.registers.append(self)
        self.id = -1
        self.name = name
        self.initialValue = None
        self.valueType = IValueType(False, 32)
        self.isConst = False
        self.table = table

    def Write(self, writer):
        writer.ofh.write("    (REGISTER " + str(self.id) + " ")
        if self.name:
            writer.ofh.write(self.name)
        else:
            writer.ofh.write("()")
        writer.ofh.write("\n")
        writer.ofh.write("     ")
        if self.isConst:
            writer.ofh.write("CONST")
        else:
            writer.ofh.write("REG")
        writer.ofh.write(" ")
        self.valueType.Write(writer)
        writer.ofh.write(" ")
        if self.initialValue:
            self.initialValue.Write(writer)
        else:
            writer.ofh.write("()")
        writer.ofh.write("\n")
        writer.ofh.write("    )\n")

    def SetInitialValue(self, int_val):
        self.initialValue = IValue(int_val)

    def SetType(self, is_signed, width):
        self.valueType = IValueType(is_signed, width)

class IResource(object):
    def __init__(self, table, resource_class):
        self.table = table
        self.resource_class = resource_class
        self.id = -1
        self.array = None
        self.input_types = []
        self.output_types = []
        self.resource_params = ResourceParams()
        self.foreign_reg = None
        self.callee_table = None

    def Write(self, writer):
        writer.ofh.write("   (RESOURCE " + str(self.id))
        writer.ofh.write(" " + self.resource_class.name + "\n")
        # input, output, params
        writer.ofh.write("    ")
        self.writeWidths(writer, self.input_types)
        writer.ofh.write(" ")
        self.writeWidths(writer, self.output_types)
        writer.ofh.write("\n")
        writer.ofh.write("    ")
        self.resource_params.Write(writer)
        if self.array:
            self.array.Write(writer)
        if self.foreign_reg:
            writer.ofh.write("    (FOREIGN-REG " + str(self.foreign_reg.table.id) + " ")
            writer.ofh.write(str(self.foreign_reg.id) + ")\n")
        if self.callee_table:
            tab = self.callee_table
            writer.ofh.write("    (CALLEE-TABLE " + tab.module.name + " ")
            writer.ofh.write(str(tab.id) + ")\n")
        writer.ofh.write("   )\n")

    def writeWidths(self, writer, types):
        writer.ofh.write("(")
        writer.ofh.write(" ".join([" UINT " + str(t.width) for t in types]))
        writer.ofh.write(")")

class IResourceClass(object):
    def __init__(self, name):
        self.name = name

class IValue(object):
    def __init__(self, int_val):
        self.val = int_val

    def Write(self, writer):
        writer.ofh.write(str(self.val))

class IValueType(object):
    def __init__(self, isSigned, width):
        self.width = width
        self.isSigned = isSigned

    def Write(self, writer):
        if self.isSigned:
            writer.ofh.write("INT")
        else:
            writer.ofh.write("UINT")
        writer.ofh.write(" " + str(self.width))

class IArray(object):
    def __init__(self, address_width, data_type, is_external, is_ram):
        self.address_width = address_width
        self.data_type = data_type
        self.is_external = is_external
        self.is_ram = is_ram

    def Write(self, writer):
        writer.ofh.write("    (ARRAY " + str(self.address_width))
        writer.ofh.write(" UINT " + str(self.data_type.width))
        if self.is_external:
            writer.ofh.write(" EXTERNAL")
        else:
            writer.ofh.write(" INTERNAL")
        if self.is_ram:
            writer.ofh.write(" RAM")
        else:
            writer.ofh.write(" ROM")
        writer.ofh.write(")\n")

class ResourceParams(object):
    def __init__(self):
        self.params = dict()

    def AddValue(self, key, value):
        if not key in self.params:
            self.params[key] = []
        self.params[key].append(value)

    def Write(self, writer):
        writer.ofh.write("(PARAMS ")
        kvs = []
        for k, values in self.params.items():
            kv = "(" + k + " " + (" ".join(v for v in values)) + ")"
            kvs.append(kv)
        writer.ofh.write(" ".join(kv for kv in kvs))
        writer.ofh.write(")\n")

# IChannel
    
class DesignWriter(object):
    def __init__(self, design):
        self.design = design
        self.ofh = sys.stdout

    def Write(self):
        self.design.Write(self)
