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

* P1: Register data type
* P1: Document
* P1: More examples
* P2: Values wider than 64bits
* P2: Pluggable optimizer and writer
* P2: Error handling

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

