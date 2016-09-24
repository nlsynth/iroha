# Iroha: Intermediate Representation Of Hardware Abstraction
Author: Yusuke TABATA (tabata.yusuke@gmail.com) and the team (TBD)

TL;DR: Aiming to be LLVM for HLS

-- Build and use

(0) First of all, build the binary.

    > $ ./configure; make
You may also have to (install automake, autoconf, libtool and) set up autotools if you got this package from git.

    > $ autoreconf

(1) Try to read Iroha IR in S-expression and just dump it.

    > $ ./iroha tests/min.iroha
    (or
    > $ ./iroha -v tests/min.iroha
    to get verilog output)


(2) Example to generate Verilog from Python examples.

    > $ cd examples
    > $ python loop.py | ../iroha - -v -o loop.v
    > (or other .py files)

(3) Examples to build Iroha IR from C++.

    > $ cd examples
    > $ python config-examples.py
    > $ make
    > $ ./minimum

(4) Example to use optimizers

    > $ ./examples/xorshift | ./iroha - -opt wire_insn,clean_empty_state,clean_unreachable_state | ./iroha - -v -o /tmp/xorshift.v


Please read src/iroha/iroha.h, src/main.cpp examples/ for more details.

-- TODOs

* P1: Document
* P1: Add name for CHANNEL
* P1: Test and debug SSA conversion and PHI removal
* P1: Set constraint not to split related insns
* P1: Values wider than 64bits
* P1: Pluggable optimizer and writer
* P1: Better error handling
* P1: I/O for embedded verilog module
* P1: Modularize mapped resource
* P1: Resource sharing between tables
* P1: Decide module/instance relation
* P2: Write shared register in another module
* P2: Simplify wait cycle if there's 1 multi cycle insn in a state
* P2: More debug output from optimizers.
* P2: Support profiling information from frontends.
* P2: Export optimizer API so that users can add own phase
* P2: API to create copies of module, table and so on.
* P3: Set reset polarity to embedded module.
* P3: Allow multiple embedded module instances.
* P3: Carry bit of adder as input/output.
* P3: Use profiling information to estimate IPC.
* Q: Do we need instance/module separation?

-- Source tree

* src/
    * Libraries and commands.
* src/builder
    * Code to build Iroha data structures.
* src/design
    * Code to manipulate design IR.
* src/iroha
    * Public header files of libraries manipulate on Iroha data structures.
    * Basic API layer code.
* src/opt
    * Optimizers
* src/writer
    * Code to write design IR as a file.
* tests/
    * Test input.

-- Authors and friends

* Takefumi Miyoshi (miyo)
* Shinya Takamaeda-Yamazaki (shtaxxx)
* Ichiro Kawazome (ikwzm)
and
* Yusuke Tabata (yt76)

-- format this document

$ markdown README.md > README.html

