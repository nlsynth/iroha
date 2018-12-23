import sys
sys.path.append('../py')

from iroha.native import native

d = native.Read("/tmp/a.iroha")
w = native.CreateWriter(d)
w.setLanguage("verilog")
w.write("/tmp/z.v")
