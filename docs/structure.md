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


## Grammar

Structure of a design in Iroha is described in following BNF.

design          := params? (channel | module)*

channel         := "(" "CHANNEL" channel-id:number value-type reader-ep:channel-ep writer-ep:channel-ep params")"

channel-ep      := "(" module-id:number table-id:number resource-id:number ")"

module          := "(" "MODULE" module-id:number module-name:label params parent-module table* ")"

parent-module   := "(" "PARENT" module-id:number ")"

table           := "(" "TABLE" table-id:number registers resources inital-state? state* ")"

inital-state    := "(" "INITIAL" state-id:number ")"

registers       := "(" "REGISTERS" register* ")"

register        := "(" "REGISTER" reg-id:number reg-name:label register-class reg-type:value-type initial-value? ")"

register-class  := "REG" | "CONST" | "WIRE"

initial-value   := number

resources       := "(" "RESOURCES" resource* ")"

resource        := "(" "RESOURCE" res-id:number res-class:resource-class input-types:value-types output-types:value-types params? resource-option? ")"

resource-class  := "tr" | "set" | "print" | "phi" | "select" | "assert" | "mapped" |
                   "channel-write" | "channel-read" |
                   "sibling-task" | "sibling-task-call" | "sub-module-task" | "sub-module-task-call" |
                   "ext-input" | "ext-output" |
                   "array" | "embedded" | "foreign-reg" |
                   "add" | "sub" | "mul" |
                   "bit-and" | "bit-or" | "bit-xor" | "bit-inv" |
                   "bit-sel" | "bit-concat" |
                   "gt" | "gte" | "eq" | "shift"

resource-option := array-desc | foreign-reg-desc | callee-task-desc

array-desc      := "(" "ARRAY" addr-width:number data-type:value-type array-ownership array-mem-type ")"

array-ownership := "EXTERNAL" | "INTERNAL"

array-mem-type  := "RAM" | "ROM"

callee-task-desc:= "(" "CALLEE-TABLE" module-id:number table-id:number ")"

foreign-reg-desc:= "(" "FOREIGN-REG" module-id:number table-id:number reg-id:number ")"

value-types     := "(" value-type* ")"

value-type      := "(" "INT"  width:number ")" |
                   "(" "UINT" width:number ")"


state           := "(" "STATE" state-id:number instruction* ")"

instruction     := "(" "INSN" insn-id:number res-class:resource-class res-id:number operand:number-list target-state-ids:number-list input-reg-ids:number-list output-reg-ids:number-list ")"

params          := "(" "PARAMS" param* ")"

param           := "(" param-key param-value ")"

param-key       := label

param-value     := label | number

number-list     := "(" number* ")"

number          := [0-9]+

label           := [a-zA-Z_]+[a-zA-Z0-9_-]*



## Design

A design contains (1) modules, (2) channels connect module to module or outside of design and (3) parameters of the design.

e.g.

(PARAMS (RESET-POLARITY true) ..)
(CHANNEL ...)
(MODULE module-id name ...)

-- Module

Module can contain multiple FSMs (tables).

e.g.

(MODULE 1 sub_mod
 (PARAMS)
 (PARENT parent_mod)
 (TABLE table-id ...)
 (TABLE table-id ...)
)

-- Channel

Channel has a reader and a writer.

e.g.

; table to another table

(CHANNEL 1 (UINT 32) (1 2 1) (1 1 1))

; reader in a module and reads from outside of the design

(CHANNEL 1 (UINT 32) (1 2 1) ())

-- Table

Table represents an FSM.

e.g.

(TABLE 1
 (REGISTERS)
 (RESOURCES)
 (INITIAL 1)
 (STATE 1 ...)
 (STATE 2 ...)
 ...)

-- Resource

e.g.

(RESOURCE 1 bit-inv
 (UINT 32)
 (UINT 32))
 (PARAMS)
 )

-- Register

Register can have one of following classes

* REG
 * Normal register
* CONST
 * Constant value
* WIRE
 * Register which can be used only in a state

e.g.

(REGISTER 12 r_a
 REG (UINT 32) 0)

-- State

State can have multiple instructions.

e.g.

(STATE 1
 (INSN ...)
 (INSN ...))

-- Insn

Instruction uses a resource and takes following arguments

1. Input registers
2. Output registers
3. Target states, if the resource is 'tr' (transition)
4. Operand to specify the detail of operation


e.g.

(INSN
 1
 logic-inv
 4
 () ; operand
 () ; target states
 (11) ; input registers
 (22) ; output registers

-- Params (design)

(PARAMS)

## format this document

$ markdown structure.md > structure.html
