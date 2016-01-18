# Iroha: Internal Representation Of Hardware Abstraction
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

(2) Examples to build Iroha IR from C++.

    > $ cd examples
    > $ python config-examples.py
    > $ make
    > $ ./minimum

Please read src/iroha/iroha.h, src/main.cpp examples/ for more details.

-- TODOs

* P0: Document
* P1: Optimizer debug output for intermediate IR
* P1: Sub module task
* P1: In design channel
* P1: Memory or array interface synth implementation
* P1: Verilog writer to support insn chaining
* P2: Set constraint not to split related insns
* P2: Delete API objects automatically
* P2: ROM image data structure.
* P2: Values wider than 64bits
* P2: Pluggable optimizer and writer
* P2: Error handling
* P2: Pipeline can be built with multiple tables in a module OR multiple modules with one table in each module
* P2: I/O for embedded verilog module
* P2: Resource sharing between tables
* P3: SSA conversion
* P3: Export optimizer API so that users can add own phase
* P3: API to create copies of module, table and so on.
* P3: C++ writer or interpreter

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

-- format this document

$ markdown README.md > README.html

