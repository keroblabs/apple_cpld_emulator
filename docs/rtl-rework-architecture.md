# RTL co-simulation rework (branch rtl-rework)

## Scope

This is a C++ simulation-adapter change, **not** an RTL redesign. The
combined Verilator `VIOU_MMU_tb` remains authoritative for the current
MMU/IOU wiring. The C-based video controller is unaffected.

## Responsibilities

- `rtl_model.cpp`: reset, one Verilator `eval()` per scheduler tick,
  and the synchronous 65816-facing compatibility wrapper.
- `rtl_cpu_bus.cpp`: CPU address/data direction and read-data capture.
- `rtl_memory_bus.cpp`: memory transaction phases and PHI_0 generation.
- `rtl_spi_slave.cpp`: independent serial bit/edge state machine.
- `rtl_interconnect.cpp`: sample the Verilog testbench's MMU and IOU
  outputs into the emulator data structures.
- `spi_controller.c`: retain the existing request/response queue,
  consuming the new SPI state machine's DONE indication.

The CPU and physical MMU addresses must **never** share a field. Each
CPU request performs four low-PHI ticks, four high-PHI ticks and one
falling-edge settle tick. The caller's CPU model completes its bus
operation only after the transaction's DONE state is observed.

SPI uses four scheduler ticks per half-clock, with a CS-high rising
clock before the 13-bit transfer, and sampling of MISO before the
shift-register-advancing clock edge. SPI proceeds while CPU bus
transactions are being evaluated. The current integration still advances
its virtual clock only when the board schedules a CPU access; truly
independent wall-clock or DMA-driven scheduling is future work. Its completion is sticky until the
existing queue consumes it.

## Transitional limitation

The 65816 emulator presently exposes a synchronous bus-access API.
`rtl_model_iterate` internally runs the state machines to completion
for each access. This provides modularity without changing the
CPU emulator's execution/complete contract. A future independent
master-clock scheduler can use the same processors and implement
CPU wait states without rewriting their internals.

The `SPLIT_CPLD` build variant has not been converted. Use the
combined `VIOU_MMU_tb` model until separate Verilator chip modules
and their explicit pin-level interconnect are revisited.

## Verification required before merging

1. Build with the same Verilator-generated testbench used by main.
2. Boot the emulator and verify ROM fetch and normal RAM reads/writes.
3. Verify soft-switch MMU mappings, especially 80STORE/PAGE2/HIRES,
   auxiliary memory reads/writes, and language-card banks.
4. Exercise slot-6 I/O, C6xx ROM, C0xx device selection and speaker.
5. Run keyboard traffic through the SPI request/response queue; verify
   all 13 serial bits, including the final data bit, and back-to-back
   commands. Inspect the IOU SPI RTL's edge behaviour if results differ.
6. Compare boot execution and SPI transaction traces with main at
   common CPU-access boundaries.
7. Confirm no deadlock when the CPU enters its existing throttle/halt
   path. The adapter should not run the CPU's complete step early.

Current status: integration implementation, **not yet build-tested or
runtime-verified** in the user's development environment.
