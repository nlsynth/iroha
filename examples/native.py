# Please make sure using python3
import sys
sys.path.append('../py')

from iroha.native import native

d = native.read("/tmp/a.iroha")
w = native.createWriter(d)
w.setLanguage("verilog")
w.write("/tmp/z.v")
