# Iroha: Intermediate Representation Of Hardware Abstraction
Author: Yusuke TABATA (tabata.yusuke@gmail.com) and the team (TBD)

TL;DR: Aiming to be LLVM for HLS

TBD! WIP!

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

* P0: Document
* P1: Multi cycle insn.
* P1: Sub module task
* P1: In design channel
* P1: Memory interface synth implementation
* P2: Test and debug SSA conversion and PHI removal
* P2: Set constraint not to split related insns
* P2: ROM image data structure.
* P2: API to create ROM
* P2: Values wider than 64bits
* P2: Pluggable optimizer and writer
* P2: Error handling
* P2: I/O for embedded verilog module
* P2: Modularize mapped resource
* P2: Resource sharing between tables
* P3: More debug output from optimizers.
* P3: Export optimizer API so that users can add own phase
* P3: API to create copies of module, table and so on.
* Q: C++ writer or interpreter?
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

-- Glossary

* Exclusive/Light resource
    * Insn which uses an exclusive resource occupies it in the state (e.g. adder).
    * Multiple insns in a same state can use a same light resource (e.g. assign).
* Insn
    * Instruction
* BB
    * Basic block
* Mapped resource
    * Resource which has actual implementation
    * e.g. Array access might be mapped to AXI, SRAM or so on.

-- format this document

$ markdown README.md > README.html

