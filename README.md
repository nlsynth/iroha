# Iroha: Intermediate Representation Of Hardware Abstraction
Author: Yusuke TABATA (tabata.yusuke@gmail.com) and the team (TBD)

TL;DR: Aiming to be LLVM for HLS

## What's this?

Iroha is a text representation of a network off state machines and tools to process it. The representation uses S-expression to describe a network of state machines.
It looks like as follows:

    (MODULE ...
      (TABLE
        (REGISTERS (REGISTER ...) (REGISTER ...) ...)
        (RESOURCES (RESOURCE ...) (RESOURCE ...) ...)
        (STATE (INSN ...) (INSN ...) ...))
      (TABLE ...))

A TABLE denotes a state machine. It can have its behavior including calculation , communication to other TABLEs and so on.

## Build and use

(0) First of all, build the binary.

    Requirements:
    C++11 capable compiler: g++ or clang++.
    Build tools: make and gyp.

    > $ ./configure ; make

(1) Try to read Iroha IR in S-expression and just dump it.

    > $ ./iroha tests/min.iroha
    (or
    > $ ./iroha -v tests/min.iroha
    to get verilog output)


(2) Example to generate Verilog from Python examples.

    > $ cd examples
    > $ python loop.py | ../iroha - -v -o loop.v
    > (or other .py files)

(3) Example to use optimizers

    > $ ./examples/xorshift | ./iroha - -opt wire_insn,clean_empty_state,clean_unreachable_state | ./iroha - -v -o /tmp/xorshift.v


Please read src/iroha/iroha.h, src/iroha/iroha_main.cpp examples/ for more details.

## TODOs

* WIP: Document
* P1: Better error handling
* P1: More debug output from optimizers.
* P1: Ability to merge AXI channels.
* P1: Connect FIFO to a dataflow table.
* P2: Export optimizer API so that users can add own phase
* P2: FPGA style register initialization
* P2: I/O for embedded verilog module
* P2: Simplify wait cycle if there's 1 multi cycle insn in a state
* P2: Set reset polarity to embedded module.
* P2: Allow multiple embedded module instances.
* Q: Do we need instance/module separation?
* AXI: Allow or fail to attach both master/slave to an array.

## Source tree

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

## Authors and friends

* Takefumi Miyoshi (miyo)
* Shinya Takamaeda-Yamazaki (shtaxxx)
* Ichiro Kawazome (ikwzm)
and
* Yusuke Tabata (yt76)

-- format this document

$ markdown README.md > README.html

