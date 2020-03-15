# IROHA structures and grammar

## Overview

A circuit design in Iroha contains following elements.

* Design
    * Everything in an HDL design.
    * consists of a tree of modules.
* Platform
    * Chip dependent parameters for the design.
    * Mainly delay information of each operation is stored.
* Module
    * An HDL module.
    * consists of sub modules and tables.
* Table
    * A state machine.
    * consists of resources and states.
* Resource
    * Various kinds of operation like numeric operation, memory access and so on.
* State
    * A state in a state machine.
    * consists of instructions.
* Insn (instruction)
    * An operation performed in a state.


## Grammar

Structure of a design in Iroha is described in following BNF.

design          := params? (platform | array-image | module)*

module          := "(" "MODULE" module-id:number module-name:label params parent-module table* ")"

parent-module   := "(" "PARENT" module-id:number ")"

table           := "(" "TABLE" table-id:number table-name:label registers resources inital-state? state* ")"

inital-state    := "(" "INITIAL" state-id:number ")"

registers       := "(" "REGISTERS" register* ")"

register        := "(" "REGISTER" reg-id:number reg-name:label register-class reg-type:value-type initial-value params?")"

register-class  := "REG" | "CONST" | "WIRE"

initial-value   := number | "(" ")"

resources       := "(" "RESOURCES" resource* ")"

resource        := "(" "RESOURCE" res-id:number res-class:resource-class input-types:value-types output-types:value-types params? resource-option? ")"

resource-class  := "tr" | "set" | "print" | "phi" | "select" | "assert" | "mapped" |
                   "sibling-task" | "sibling-task-call" | "sub-module-task" | "sub-module-task-call" |
                   "ext-input" | "ext-output" |
                   "array" |
                   "add" | "sub" | "mul" |
                   "bit-and" | "bit-or" | "bit-xor" | "bit-inv" |
                   "bit-sel" | "bit-concat" |
                   "gt" | "gte" | "eq" | "shift" | ...

resource-option := array-desc | callee-task-desc

array-desc      := "(" "ARRAY" addr-width:number data-type:value-type array-ownership array-mem-type array-image-id:number ")"

array-ownership := "EXTERNAL" | "INTERNAL"

array-mem-type  := "RAM" | "ROM"

array-image     := "(" "ARRAY-IMAGE" array-image-id:number array-image-name:label "(" value:number* ")" ")"

callee-task-desc:= "(" "CALLEE-TABLE" module-id:number table-id:number ")"

value-types     := "(" value-type* ")"

value-type      := "(" "INT"  width:number ")" |
                   "(" "UINT" width:number ")"


state           := "(" "STATE" state-id:number profile? instruction* ")"

instruction     := "(" "INSN" insn-id:number res-class:resource-class res-id:number operand:number-list target-state-ids:number-list input-reg-ids:number-list output-reg-ids:number-list depending-instruction-ids:number-list ")"

params          := "(" "PARAMS" param* ")"

param           := "(" param-key param-value ")"

param-key       := label

param-value     := label | number

platform        := "(" "PLATFORM" label platform-def* ")"

platform-def    := "(" "DEF" platform-cond platform-value ")"

platform-cond   := "(" "COND" platform-node ")"

platform-value  := "(" "VALUE" platform-node ")"

platform-node   := "(" label ( platform-node | label | number )* ")"

profile         := "(" "PROFILE" ")" | "(" "PROFILE" number number? ")"

number-list     := "(" number* ")"

number          := [-]?[0-9]+

label           := [a-zA-Z_]+[a-zA-Z0-9_-]*

label-or-empty  := label | "(" ")"



## Design

A design contains modules and parameters of the design.

e.g.

(PARAMS (RESET-POLARITY true) ..)
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

-- Profile

Profile represents the number of times the corresponding state is executed.
This can take a raw count and a normalized count (optional).

e.g.

(PROFILE 123 10)


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
 ())  ; depending instructions

-- Platform

Platform describles characteristics (mainly deley of operators) of a chip.
Each definition has a condition and evaluated on each look up.


e.g.

(PLATFORM generic
 (DEF (COND (CLASS add)) (VALUE (DELAY 2000))))

-- Params (design)

## format this document

$ markdown structure.md > structure.html
