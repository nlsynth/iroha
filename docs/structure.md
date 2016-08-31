# IROHA structures and grammar

## Overview

A circuit design in Iroha contains following elements.

* Design
    * Everything in an HDL design.
    * consists of a tree of modules.
* Module
    * An HDL module.
    * consists of sub modules and tables.
* Table
    * A state machine.
    * consists of resources and states.
* Resource
    * Various kinds of operation like numeric operation, memory access and so on.
* Channel
    * connects a resource to another resource in the design or outside of the design.
* State
    * A state in a state machine.
    * consists of instructions.
* Insn (instruction)
    * An operation performed in a state


## Design

Top level

Design := PARAMS? (CHANNEL | MODULE)*

e.g.
(PARAMS (RESET-POLARITY true))
(MODULE module-id name ...)

-- Module

(MODULE module-id name
 (PARAMS)
 (PARENT parent-name)
 (TABLE table-id ...))

-- Channel

e.g.
(CHANNEL 1 (UINT 32) (1 2 1) (1 1 1))

(CHANNEL channel-id value-type reader-ep writer-ep)


-- Table

(TABLE table-id
 (REGISTERS)
 (RESOURCES)
 (INITIAL state-id)
 (STATE state-id)
 (STATE ...)
 ...)

-- Registers

(REGISTERS
 (REGISTER ...)
 (REGISTER ...)
)

-- Resources

(RESOURCE resource-id resource-class
 (input-types)
 (output-types)
 (params)
 (array-desc) | (foreign-reg-desc) | (callee-task-desk)
 )

-- Register

(REGISTER register-id name
 reg-class reg-type reg-type-param initial-value)

reg-class: REG, CONST, WIRE

-- State

(STATE state-id
 (INSN ...)
 (INSN ...))

-- Insn

(INSN
 insn-id
 resource-class
 resource-id
 (operand)
 (target-state-ids)
 (input-reg-ids)
 (output-reg-ids))

-- Params (design)

(PARAMS)

## format this document

$ markdown structure.md > structure.html
