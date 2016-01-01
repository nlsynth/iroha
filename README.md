# Iroha: Internal Representation Of Hardware Abstraction
Author: Yusuke TABATA (tabata.yusuke@gmail.com) and the team (TBD)

TL;DR: Aiming to be LLVM for HLS

TBD! WIP!

-- Build and use

(0) First of all, build the binary.

    > $ ./configure; make
You may also have to (install and) run auto tools if you got this package from git.

    > $ aclocal; automake; autoconf

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

* P1: Implementation and example of sub module
* P1: Implementation and example of channel
* P1: More examples
* P1: Document
* P1: Low level synth phase
* P1: Memory or array interface synth implementation
* P1: Optimizer
* P1: Assert module
* P1: (Verilog writer) Support insn chaining.
* P2: Delete API objects automatically
* P2: ROM image data structure.
* P2: Values wider than 64bits
* P2: Pluggable optimizer and writer
* P2: Error handling
* P2: Pipeline can be built with multiple tables in a module OR multiple modules with one table in each module
* P2: I/O for embedded verilog module
* P2: Resource sharing between tables
* P2: Export optimizer API so that users can add own phase
* P2: API to create copies of module, table and so on.

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

-- format this document

$ markdown README.md > README.html

