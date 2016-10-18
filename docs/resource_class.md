# Iroha resource classes

## Overview

Resource represents an operation and its circuit.

Various kinds of operation can be represented as a resource including

* Logic or arithmatic operators
* Non synthesizable operators (can be transformed to synthesizable)
* In circuit communications
* Adhoc information storage

and so on.

Resources are described like this and each resource are associated to a resource class.

    (RESOURCE
      ; resource id
      123
      ; resource class name
      add
      ; input types
      ((UINT 32) (UINT 32))
      ; output types
      ((UINT 32))
      ; parameters mainly for RTL generation
      (PARAMS))

Each insn uses a resource in a state like this.

    (INSN
      ; insn id
      456
      ; resource class
      add
      ; resource id
      123
      ; operand
      ()
      ; next states
      ()
      ; input register ids
      (1 4)
      ; output register ids. e.g. r2 <= add(r1, r4)
      (2))


## Details

### State transition

#### tr (transition)

Specifies next state(s) to be transitioned.

This can take a 1 bit register to specifiy conditional transition to 2 states. If no condition is specified, the insn specifies 1 next state to be transitioned unconditionally.

TODO: Decide how to handle multi bit condition.

### Arithmatic operations

#### set

Assigns the value of input register to output register.

#### select

Takes 3 input values; one is 1 bit condition and outputs 2nd or 3rd value depending on the condition value.

#### add

Adds 2 input values and outputs the result.

#### sub

Subtracts 2 input values and outputs the result.

#### mul

Multiplies 2 input values and outputs the result.

#### gt

Compares 2 inputs and outputs 1 if input[0] > input[1].

#### gte

Compares 2 inputs and outputs 1 if input[0] >= input[1].

#### eq

Compares 2 inputs and outputs 1 if input[0] == input[1].

#### shift

Shift input[0] by input[1] bits to the direction specified by operand.
input[1] should be a constant.

#### bit-and

Applies bit AND operation to 2 inputs and outputs the result.

#### bit-or

Applies bit OR operation to 2 inputs and outputs the result.

#### bit-xor

Applies bit XOR operation to 2 inputs and outputs the result.

#### bit-inv

Invert bits of 1 input and outputs the result.

#### bit-sel

TBD.

#### bit-concat

TBD.

### Pseudo instructions

Pseudo resources are resources which can't be translated into synthesizable RTL.

#### pseudo

Pseudo insn can be used inside frontend and optimization passes.

Other passes may not be able to accept a design with pseudo resource and insns using it, so it is recommended to remove pseudo resource at some phase.

#### phi

Phi appears in optimzation passes use SSA form.

#### print

Takes 1 inputs and displays the result ($display in Verilog)

#### assert

Takes 1 input and displays error message if it is false.


### Resource access

#### array

Array represents internal or external memory (either ROM or RAM).

Array itself isn't synthesizable and should be converted to mapped resource.

### Synthesizable

#### mapped

Mapped resource can be used in RTL generation phase. This can represent actual bus protocol, pin names and so on.

For now, this is used only for SRAM I/F to internal and external memory.

#### ext-input

Reads from input pin connected to outside of the design.

#### ext-output

Writes to output pin connected to outside of the design.

#### embedded

Embed a Verilog module. Insn pass values to the module, if it accepts them.

### Communication

#### channel-write

Writes input value to the channel.

#### channel-read

Reads value from the channel.

#### sibling-task-call

Kicks the specified state machine in sibling-task table.

#### sub-module-task-call

Kicks the specified state machine in sub-module-task table.


#### foreign-reg

Reads a register in another table. The register can be accessed from multiple tables.

#### shared-reg

This resource holds a value and can be read or written from other tables via shared-reg-reader(s) and shared-reg-writer(s).
It also can be read or written from the belonging table.
If writes come from multiple shared-reg-writers, arbitration (fixed priority for now) selects one value.

#### shared-reg-writer

Attach to a shared-reg in another table and allow to write to it via this resource.
Multiple shared-reg-writer can write a same shared-reg.

#### shared-reg-reader

Attach to a shared-reg in another table and allow to read it via this resource.
Multiple shared-reg-writer can read from a same shared-reg.

### Table type modifier

If there is an insn using following resource in the initial state, the meaning of table will be different from normal state machine.

#### sibling-task

The table will be configured as a task. Other tables in this module can kick this table.

#### sub-module-task

The table will be configured as a task. Other tables in super modules can kick this table.

#### dataflow-in

The table will be configured as a data flow pipeline instead of a state machine.
