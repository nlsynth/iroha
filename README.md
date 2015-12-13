# Iroha: Internal Representation Of Hardware Abstraction
Author: Yusuke TABATA (tabata.yusuke@gmail.com) and the team (TBD)

TL;DR: Aiming to be LLVM for HLS

TBD! WIP!

-- Build and use

(0) First of all, build the binary.

    > $ ./configure; make

(1) Try to read Iroha IR in S-expression and just dump it.

    > $ ./iroha tests/min.iroha

(2) Examples to build Iroha IR from C++.

    > $ cd examples
    > $ python config-examples.py
    > $ make
    > $ ./minimum

Please read src/main.cpp and src/iroha/iroha.h for more details.

-- TODOs

* P0: End to end features other than optimizers
* P1: Write an output module for a language HTML? Verilog?
* P1: API to build from IR C++.
* P2: Memory management
* P2: Document
* P3: Error handling

-- Source tree

* src/
    * Libraries and commands.
* src/builder
    * Code to build Iroha data structures.
* src/iroha
    * Public header files of libraries manipulate on Iroha data structures.
    * Basic API layer code.
* tests/
    * Test input.

-- format this document

$ markdown README.md > README.html

