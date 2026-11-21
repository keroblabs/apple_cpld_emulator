# Apple CPLD Emulator — SPI transport and IOMEM command protocol

**Status:** Current wire protocol documented; higher-level additions explicitly proposed, not implemented.  
**Baseline:** `rtl-rework`, 20 November 2026.  
**Companion:** [IOMEM command implementation reference](iomem-command-implementation-reference.md). See also [RTL rework architecture](rtl-rework-architecture.md).

## 1. Purpose and design boundaries

The existing IOU Verilog offers a **bidirectional, single-byte mailbox** to Apple software. An emulated microcontroller/controller exchanges those mailbox bytes over SPI, and `iomem_controller.c` interprets them as device commands and replies.

There are three deliberately separate layers:

1. **Apple-visible mailbox ABI:** CPU reads and writes to `$C0F1`, `$C0F2`, and `$C0F3`, with one pending bit per direction.
2. **Internal SPI transport:** thirteen-bit serial words between the controller and `IOU_spi_iface.v`. Its register selector addresses a mailbox, **not** a device or an IOMEM command.
3. **IOMEM command protocol:** byte-oriented opcode, parameters, acknowledgement, result and response payload. Command implementations must not depend on SPI framing.

We can implement mouse, RTC, disk or logical SPI-flash operations through this existing mailbox **without modifying Verilog**. A genuinely different, more efficient SPI physical framing would require reworking `IOU_spi_iface.v`; such a change should preserve the Apple ABI and the command stream.

The existing hard-drive implementation in `emulator/slot_peripherals/hard_drive.c` is a separate slot-I/O implementation, **not** an IOMEM command handler.

## 2. Source-level architecture

| File | Responsibility |
| --- | --- |
| `rtl_IOU_MMU_TB/IOU_MMU_tb.v` | Combined IOU/MMU testbench connects the mailbox and SPI modules |
| `rtl_IOU/IOU_iomem.v` | CPU-side C0Fx mailbox, data registers and handshake bits |
| `rtl_IOU/IOU_spi_iface.v` | 13-bit serial exchange, status shift register, and pending-bit triggers |
| `rtl_IOU/IOU_toggle_jk.v` | PHI_0-clocked state transitions for pending flags |
| `cpld/rtl_spi_slave.cpp` | Controller-side SPI clock, MOSI/MISO and transaction state machine |
| `cpld/rtl_model.cpp` | Central Verilator evaluation scheduler |
| `emulator/onboard/spi_controller.c` | Shared SPI request/response queue and transaction IDs |
| `emulator/onboard/iomem_controller.c` | Command assembly, lookup, ACK and outgoing reply state machine |
| `emulator/onboard/keyboard_controller.c` | Existing working keyboard producer and acknowledgement pattern |
| `emulator/onboard/board.c` | Calls SPI/IOMEM processors while servicing CPU bus cycles |

Although called `rtl_spi_slave.cpp`, the C++ component **drives the master-side SPI signals**; the Verilog `IOU_spi_iface` is the actual SPI slave.

## 3. Apple-facing C0Fx mailbox

In `IOU_iomem.v`, once C0Fx is selected, `ADDR[1:0]` determines the operation.

| Canonical address | Direction | Data / action |
| --- | --- | --- |
| `$C0F1` | CPU write | Latch the next Apple→controller byte; set `reg_wr_pending` |
| `$C0F1` | CPU read | Return `{reg_wr_pending,7'b0}`; bit 7 = pending |
| `$C0F2` | CPU read | Return next controller→Apple byte; clear `reg_rd_pending` |
| `$C0F3` | CPU read | Return `{reg_rd_pending,7'b0}`; bit 7 = pending |
| `$C0F0` | either | No command-mailbox action defined; do not use as protocol data |

F5/F6/F7, F9/FA/FB, and FD/FE/FF currently alias the respective F1/F2/F3 operations because the RTL decodes only the low two bits within C0Fx. **Use F1/F2/F3 as canonical addresses**; aliases are not a public compatibility promise.

### 3.1 Apple → controller

1. Apple polls `$C0F1 & $80` until zero (mailbox empty).
2. Apple writes exactly one byte at `$C0F1`; the pending bit becomes one.
3. Controller polls SPI status, then issues selector 0 to read the byte.
4. On SPI consume, the Verilog clears `reg_wr_pending` (through a PHI_0-clocked pending-flag circuit).
5. Apple must wait for the bit to clear before writing another byte.

### 3.2 Controller → Apple

1. Controller issues SPI selector 1 with the next reply byte.
2. Verilog latches the byte and raises `reg_rd_pending`.
3. Apple polls `$C0F3 & $80` until nonzero.
4. Apple reads `$C0F2` exactly once to receive/acknowledge that byte.
5. Controller polls SPI status until `reg_rd_pending` is clear before sending another byte.

These are **one-byte mailboxes, not FIFOs**. Neither side may overwrite a pending byte. The pending-flag logic runs on PHI_0; simply finishing a SPI serial word does not by itself establish that the Apple has consumed a reply.

### 3.3 Keyboard is independent

SPI selector 2 publishes a seven-bit keyboard code and sets `reg_kbd_pending`. Apple reads the normal C00x keyboard data/status; a C01x keyboard-strobe access clears the pending flag. Keyboard messages do not enter the IOMEM command byte stream. Status bit 2 shares the SPI status word with both IOMEM pending bits.

## 4. Existing SPI wire protocol

`spi_controller_data_start(addr, data_in)` serializes one 13-bit, MSB-first word:

```text
MOSI (controller → IOU)
    bits 12 .. 8                 bits 7 .. 0
    +-------------------------+----------------+
    | 5-bit register selector | 8-bit payload  |
    +-------------------------+----------------+

    wire_word = ((addr & 0x1f) << 8) | data_in

MISO (IOU → controller), concurrently
    12  11     10          9           8          7..0
   +---+---+-----------+-----------+-----------+---------------+
   | 0 | 0 | kbd_pend  | rd_pend   | wr_pend   | returned byte |
   +---+---+-----------+-----------+-----------+---------------+
```

The software decodes the MISO status as `flags = (received >> 8) & 0x1f`.

| SPI selector | Constant | Controller transaction | Returned lower 8 bits |
| --- | --- | --- | --- |
| 0 | `SPI_REGISTER_APL2CTRL` | Consume Apple's pending byte | The Apple-written byte |
| 1 | `SPI_REGISTER_CTRL2APL` | Post one byte for Apple | Not meaningful |
| 2 | `SPI_REGISTER_KEYBOARD` | Post keyboard keycode (7-bit field) | Not meaningful |
| 5 | `SPI_REGISTER_NONE` | Poll current pending flags only | Not meaningful |
| Others | none defined | No defined mailbox operation | Not meaningful |

Status flags: bit 0 = Apple→controller pending; bit 1 = controller→Apple pending; bit 2 = keyboard pending; bits 3–4 are zero in the inspected RTL.

At a CS-high rising SPI clock edge, `IOU_spi_iface.v` loads its status shift register. During a CS-low transaction it shifts thirteen bits; after the fifth incoming bit, selector 0 loads `reg_wr_data` into the transmit shifter, while other selectors load `8'hFF`. Consequently the trailing MISO byte on a status-only transaction is not a valid data response and must be ignored. The command interpreter never uses SPI selectors for opcodes.

`rtl_spi_slave.cpp` supplies a CS-high reset/load clock, thirteen CS-low data bits, a trailing clock and CS closing. The RTL shifts on rising SPI edges; the emulator captures MISO in the low half-period before those shifts. The current implementation advances at four simulation ticks per SPI half-period, driven by CPU-access scheduling rather than an independent wall clock. Preserve this edge behaviour until a dedicated RTL waveform test proves an alternative equivalent.

### 4.1 GPIO / bit-banged timing contract

The verified master contract is defined by signal edges, not by an STM32 SPI
peripheral mode:

1. SCK idles low with CS high. Keep CS high through at least one rising SCK
   edge; the IOU loads the five status bits and resets its frame receiver on
   that edge.
2. Pull CS low while SCK is low. Drive MOSI for each bit during the low phase,
   keep it stable through the rising edge, and send the 13-bit word MSB first:
   five selector bits followed by eight payload bits.
3. The master samples MISO during the low phase immediately before each rising
   edge. The IOU shifts on rising edges. The first five returned bits are the
   status flags; the next eight are the mailbox byte for selector 0 or `0xFF`
   for other selectors.
4. After the thirteenth data rising edge, the selected pending control is
   active low until one trailing rising edge advances the frame to its idle
   terminal count. Keep CS low for that trailing edge, then raise CS. The
   trailing edge carries no payload.
5. If CS aborts a frame, the pending controls become inactive immediately; the
   receiver state resets on the next rising edge while CS is high. Start the
   next transaction with the preload edge. Additional clocks after the
   trailing edge are ignored until that preload.

The emulator master uses four scheduler ticks per SCK half-period (eight per
complete SCK period), drives MOSI throughout each low/high pair, and captures
MISO at the end of the low phase. That is the tested digital interval; the
scheduler has no physical time unit, so it does not establish a maximum SCK in
hertz, electrical setup/hold time, or a hardware timing margin. The SPI pending
controls are sampled by PHI_0-clocked mailbox logic; the integrated Verilator
regression interleaves those clocks, while physical clock-domain timing remains
unmeasured.

### 4.2 Software queue semantics

Current `spi_data_t` (in `emulator/onboard/board.h`):

```c
typedef struct {
    uint8_t id;     // Software request ID; NOT transmitted on SPI
    uint8_t addr;   // SPI register selector
    uint8_t data;   // Transmit/receive data byte
    uint8_t flags;  // Received pending bits
} spi_data_t;
```

`spi_controller_data_start()` queues a transaction and returns an 8-bit incrementing ID; `spi_controller_check_complete(id, &reply)` returns a matching reply. IDs wrap after 256 submissions and are **local queue metadata**. The controller uses TX and RX ring buffers of `sizeof(spi_data_t) * 8` bytes each; the ring uses one spare byte/position to distinguish full and empty, so do not assume eight records fit. Existing enqueue does not report capacity failure and may silently overwrite older records if abused. This is a genuine risk for a 512-byte reply. Serialise and acknowledge every byte, and introduce checked queue backpressure before high-volume use.

SPI is shared with the keyboard; a transaction or reply may not assume exclusive use of the link. Queue request IDs must remain associated with the initiating producer.

## 5. IOMEM command engine — current source contract

The current `iomem_controller.c` contains an internal `iomem_cmd_t` with:

```c
uint8_t command;
uint16_t (*start_func)(uint8_t command);
void (*data_rx_func)(uint8_t *request, uint16_t size);
bool_t (*data_tx_func)(uint8_t *next_byte);
struct iomem_cmd_t_ *p_next;
```

The first Apple byte is the command opcode. The controller searches `iomem_cmd_list`. If unknown, it publishes **`0x55`** and returns idle. If found, it publishes **`0x60`** as a command-recognition ACK. After that ACK has been consumed, it calls `start_func(opcode)`, appends the opcode as byte zero of `iomem_request[]`, then collects additional Apple bytes until reaching the returned length. From the implementation, **the return value is total request size including the opcode**, not only the parameter count.

After request assembly, `data_rx_func(request, size)` processes all bytes; `data_tx_func(&byte)` is invoked for the first reply byte and then repeatedly after the Apple acknowledges each previously posted byte. Return true when a byte was produced, false when the response is finished. The raw request buffer has `IOMEM_MAX_REQUEST = 512 + 16 = 528` bytes.

Controller mailbox operations use SPI selector 5 to poll, selector 0 to receive a byte, selector 1 to transmit a response byte, then selector 5 to wait for the Apple to consume it.

**Limitations requiring software work before claiming IOMEM commands operational:**

1. `iomem_cmd_list` is reset to NULL during init and no public handler registration API exists. Keyboard works separately and does not prove command dispatch.
2. A zero-parameter command (total request size = 1) is not dispatched immediately; the current FSM waits for a parameter that will not arrive.
3. The returned request size is not range-checked against 1..528; the request indexing is not bounded before every write.
4. Duplicate opcodes, missing callbacks, cancellation and handler lifetime are undefined.
5. `IOMEM_COMMAND_TIMEOUT` and `IOMEM_TRANSMIT_TIMEOUT` are polling-loop counts of 100, **not milliseconds**; recovery is not specified rigorously.
6. The SPI queue can overflow without signalling the caller.
7. Application response payload format and terminal error handling are not standardised.
8. Host-side Apple routines for this mailbox are not provided by the repository.

## 6. Proposed stable IOMEM API (not implemented)

Minimal-change registration contract; these types mirror the existing internal callbacks:

```c
// Move the descriptor/callback definitions into iomem_controller.h.
typedef uint16_t (*iomem_start_func_t)(uint8_t opcode);
typedef void (*iomem_data_rx_func_t)(uint8_t *request, uint16_t size);
typedef bool_t (*iomem_data_tx_func_t)(uint8_t *next_byte);

typedef struct iomem_cmd_t_ {
    uint8_t command;
    iomem_start_func_t start_func;
    iomem_data_rx_func_t data_rx_func;
    iomem_data_tx_func_t data_tx_func;
    struct iomem_cmd_t_ *p_next;
} iomem_cmd_t;

// Proposed registration function; does not exist yet:
bool_t iomem_register_command(iomem_cmd_t *descriptor);
```

Contract:

- Call `iomem_controller_init()` once before registering handlers.
- Use static-lifetime descriptors; reject NULLs and duplicate command IDs.
- Preserve one active command at a time; do not begin a second until the first reply is finished.
- `start_func` must return a length in **1..528**, including the opcode, and is called after the command ACK has been read.
- Size 1 triggers `data_rx_func` immediately, not a wait for parameters.
- `data_rx_func` receives `request[0] = opcode` and all bytes in order.
- `data_tx_func` returns true **for each byte it produces**; false after the last byte. The mailbox driver acknowledges each byte before requesting the next.
- Handler-owned buffers live at least until the final reply is acknowledged; abort/reset must cancel an in-flight command safely.
- The registration interface must not expose SPI transaction IDs, Verilator objects, chip-select timing or physical framing.

### 6.1 Proposed standard response envelope

Keep the legacy opcode phase (`0x60` ACK / `0x55` invalid opcode) distinct from the **final result** after parameters:

```text
+------------+-----------------------+---------------------+
| status: 1B | payload length: 2B    | payload: 0..65535B |
+------------+-----------------------+---------------------+
             big-endian
```

For example, success `status=0x00`, error status nonzero; on error send zero-length payload unless a command explicitly defines error data. Suggested **not yet assigned** error statuses: `0x01` invalid arguments, `0x02` out of range, `0x03` device failure, `0x04` busy. The envelope is a **proposal**, not presently generated by `iomem_controller.c`. Initially it can be emitted from each handler; a common helper is preferable.

No new RTL register or SPI selector is required. Apple receives envelope and payload as ordinary controller→Apple mailbox bytes. The Apple driver knows when to stop from the two-byte length, **not** a sentinel byte (because 0x00 and 0xFF must be legal payload data).

## 7. Implementation and recovery requirements

- Validate request length before copying bytes; check indices on **every** write.
- Add a checked SPI-queue enqueue API; do not drop requests or replies when full.
- Keep strict byte ordering and pending-bit backpressure in both directions.
- Define Apple-side bounded waits, controller-side timeouts in actual service-time units, and a resync/abort rule on error or reset.
- Verify timing for PHI_0-clocked pending-bit clears/sets; avoid relying solely on SPI transaction completion.
- Keep mailbox/command error statuses separate from transport errors and stale data.
- Decide how controller-side asynchronous device operations signal readiness instead of blocking the emulator.
- Preserve the independent keyboard path.
- Document reservations of the C0Fx address space; unrelated Apple soft switches may otherwise conflict.
- Benchmark complete responses; one SPI word and a mailbox handshake per byte do **not** imply disk-class throughput.

A future SPI transport rewrite may bundle multiple mailbox operations **on the controller side**, without changing commands. True wire-level bursts or a different 13-bit framing require Verilog changes. The command ABI and Apple-facing registers should still be preserved.

## 8. Implementation sequence

1. Export/implement command registration and initialise then register static handlers.
2. Correct request-size=1 behaviour, validate request lengths/indices and recover from failures.
3. Add SPI backpressure/error reporting and reliable timeout/cancellation rules.
4. Write a small Apple mailbox driver and a deterministic echo command.
5. Establish the standard response envelope and register the [READ_LBA example](iomem-command-implementation-reference.md).
6. Test concurrency with keyboard, error paths, reset, repeated commands and 512-byte replies.
7. Only after correctness/measurement, decide whether SPI transport optimisation is warranted.

## 9. Test/acceptance matrix

| Scenario | Expected |
| --- | --- |
| Apple writes C0F1; controller reads selector 0 | Bit7 pending rises then clears; data preserved |
| Controller selector 1; Apple reads C0F2 | C0F3 pending rises then clears; data preserved |
| SPI selector 5 poll | Correct status bits; no mailbox side effects |
| SPI selector 2 keyboard | Existing keyboard path remains operational |
| Unknown opcode | One legacy 0x55 rejection; no parameter collection |
| Registered opcode | 0x60 ACK, correct parameter count, handler called once |
| Opcode-only command | Completes without waiting for extra bytes |
| 528-byte request / 529-byte request | First safe, second rejected before overwrite |
| 512-byte response containing 00 and FF | All bytes match, no sentinel confusion |
| Keyboard traffic interleaved with IOMEM | Correct SPI response ownership and progress |
| Full queue, timeout or mid-command reset | Explicit recoverable error, no deadlock |
| Back-to-back commands | No stale byte/pending flag, ROM/CPU boot unaffected |

**Evidence:** source-level review of `rtl-rework` and user confirmation that the earlier RTL-interface refactor runs. The proposed API, framing helper, Apple driver, command handlers and 512-byte tests are **not yet implemented or validated**.
