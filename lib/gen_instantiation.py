#!/usr/bin/env python

import sys

USAGE = """
This script generates Verilog module instantiation code for output from Iroha.
e.g.
 $ iroha a.iroha -v -s -o a.v
 $ gen_instantiation.py -w a.v m_:m00_

Usage: [OPTIONS] [Verilog file] [interface mapping]
  -w Generates wires

  [interface mapping] is a comma separated list of mappings from interface name
  prefix to a name for output.
e.g.
  m_:m00_,s_s00

  // Wires are generated, if -w is specified.
  wire [1:0] m00_X;
  wire [1:0] s00_Y;
  // The instance take connections with rewritten names like m00_X from m_X.
  module module_inst(... .m_X(m00_X), .s_Y(s00_Y) ...)

"""

def getPrefix(p, prefixes):
    if p in prefixes:
        return prefixes[p]
    return ""

def genWires(pins, prefixes):
    for pin in pins:
        # dir width prefix name
        n = "  wire"
        p = pin.split(":")
        w = int(p[1])
        if w > 0:
            n += " [" + str(w - 1) + ":0]"
        n += " " + getPrefix(p[2], prefixes) + p[3] + ";"
        print(n)

def genConnection(modinfo, pins, prefixes):
    mod = modinfo[0]
    inst = modinfo[1]
    ios = []
    for pin in pins:
        p = pin.split(":")
        n = "." + p[2] + p[3] + "(" + getPrefix(p[2], prefixes) + p[3] + ")"
        ios.append(n)
    o = "  " + mod + " " + inst + "("
    o += ", ".join(ios)
    o += ");\n"
    print(o)

def procFile(fn):
    for line in open(fn):
        line = line[:-1]
        if line.startswith("//"):
            tokens = line.split(" ")
            if tokens[1] == ":connection:":
                return tokens[2:]
    return None

def parsePrefixMap(prefixMap):
    pm = {}
    if prefixMap == "":
        return pm
    maps = prefixMap.split(",")
    for m in maps:
        kv = m.split(":")
        pm[kv[0]] = kv[1]
    return pm

ifn = ""
prefixMap = ""
flagGenWire = False


args = sys.argv[1:]
while len(args) > 0:
    a = args[0]
    if a.startswith("-"):
        if a == "-w":
            flagGenWire = True
        args = args[1:]
        continue
    if ifn == "":
        ifn = a
    elif prefixMap == "":
        prefixMap = a
    args = args[1:]

if ifn == "":
    print("Input is not specified")
    print(USAGE)
    exit(0)

prefixes = parsePrefixMap(prefixMap)

info = procFile(ifn)
modinfo = info[0].split(":")
pins = info[1].split(",")

if flagGenWire:
    genWires(pins, prefixes)

genConnection(modinfo, pins, prefixes)
