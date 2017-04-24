#! /usr/bin/python
#
# Borrowed the idea from here:
#  http://stackoverflow.com/questions/27267951/how-to-generate-make-install-action-using-gyp

import os
import sys

PREFIX="/usr/local"

for arg in sys.argv:
    if arg.startswith("--prefix="):
        PREFIX=arg[9:]

# Remove trailing / unless it's root.
while len(PREFIX) > 1 and PREFIX.endswith("/"):
    PREFIX = PREFIX[:-1]

fh = open("config.mk", "w")
fh.write("prefix=" + PREFIX + "\n")
fh.close()
os.system("make -f Makefile.gyp src/Makefile")
