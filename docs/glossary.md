# Glossary

## Exclusive/Light resource

* Insn which uses an exclusive resource occupies it in the state (e.g. adder).
* Multiple insns in a same state can use a same light resource (e.g. assign).

## Insn
* Instruction

## BB
* Basic block

## Mapped resource
* Resource which has actual implementation
* e.g. Array access might be mapped to AXI, SRAM or so on.

## Table type
* A table can either be (1) Normal state machine, (2) Task state machine or (3) Data flow.

(1) Normal state machine

Normal state machine runs continuously after reset.
e.g.

    if (rst) st <= `S_0;
    else case (st)
      `S_0: begin
        ...
        st <= `S_1;
      end
      `S_1: begin
        ..
      end
      ...
    endcase

(2) Task state machine

Task state machine starts run when it is requested to start from another table.
e.g.

    if (rst) st <= `S_idle;
    else case (st)
      `S_idle: begin
        // Task is requested to run.
        if (req) st <= `S_0;
      end
      `S_0: begin
        ...
        st <= `S_1;
      end
      `S_1: begin
        ..
      end
      ...
      `S_99: begin
        // Task is done.
        st <= `S_idle;
      end
    endcase

(3) Data flow

Data flow propagates validity of each stage to the next stage on every clock.
e.g.

    if (rst) begin
      st_0 <= 0;
      st_1 <= 0;
      st_2 <= 0;
      ...
    end else begin
      if (input_en) st_0 <= 1;
      end st_0 <= 0;
      // Stage 0
      if (st_0) begin
        ...
	// Propagates validity of this stage to the next stage.
        st_1 <= 1;
      end else st_1 <= 0;
      // Stage 1
      if (st_1) begin
        ...
        st_2 <= 1;
      end else st_2 <= 0;
      ...
    end

## Module import

Imported modules can be connected to its outer module using TAP.

(MODULE 1 mod
  (MODULE-IMPORT /tmp/a.iroha
    (TAP a_in tag_123)
    (TAP b_in ())
  )
)
