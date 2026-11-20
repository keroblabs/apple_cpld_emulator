# IOMEM command implementation reference: 512-byte READ_LBA example

**Status:** Proposed end-to-end reference, not an implementation already connected to the emulator.  
**Protocol authority:** [SPI transport and IOMEM protocol](spi-iomem-protocol.md).  
**Constraint:** Reuse existing RTL mailbox; no Verilog changes for this command.

## 1. Why this example

The Apple program sends a **logical sector address (LBA)** and receives 512 bytes. This exercises both directions of the mailbox, command registration, parameter assembly, device lookup, a large response, flow control and completion. It is also a useful model for mouse, RTC and flash-service commands.

The existing `emulator/slot_peripherals/hard_drive.c` operates through a different slot-I/O latch/FIFO interface and currently uses 16-bit block numbers. This example deliberately specifies a new **32-bit LBA** IOMEM command. It will need an appropriate backend adapter, not a silent reuse of that 16-bit API.

## 2. Proposed command definition

Example opcode reservation: **`IOMEM_CMD_READ_LBA = 0x40`**. Not present in an existing registry; avoid assuming it is already allocated.

### Request

| Byte offset | Description | Example |
| --- | --- | --- |
| 0 | Command opcode | `40` |
| 1 | Logical device number | `00` |
| 2 | LBA bits 31..24 | `00` |
| 3 | LBA bits 23..16 | `12` |
| 4 | LBA bits 15..8 | `34` |
| 5 | LBA bits 7..0 | `56` |

Full request: `40 00 00 12 34 56`. Its length is **6 bytes INCLUDING the opcode**. LBA is `0x00123456` and is encoded **big-endian**, regardless of host machine endianness.

### Acceptance and final response

The current IOMEM FSM emits legacy ACK **`60`** as soon as it recognises the opcode, **before** receiving the five parameters. Unknown opcode returns **`55`** instead, and the Apple side must not send parameters.

After the whole request, the **proposed** final response envelope is:

| Field | Size | Successful read | Failed read (out-of-range example) |
| --- | --- | --- | --- |
| Status | 1 | `00` | `02` |
| Payload length (big-endian) | 2 | `02 00` = 512 | `00 00` |
| Payload | length bytes | sector bytes 0..511 | none |
| **Total final response** | | **515 bytes** | **3 bytes** |

The 0x60 ACK is **separate** and not counted in the 515-byte result. A final status code is **not** a SPI status flag: it is an ordinary command-level byte delivered through the mailbox.

## 3. Complete logical transaction

```mermaid
sequenceDiagram
    participant A as Apple 65816 driver
    participant R as IOU C0Fx registers
    participant S as Controller SPI transport
    participant I as IOMEM dispatcher
    participant D as READ_LBA handler
    A->>R: Wait C0F1 bit7=0; write 0x40
    I->>S: Poll selector 5; consume selector 0
    S->>R: Fetch opcode; clear write pending
    S-->>I: Opcode 0x40
    I->>D: Lookup registered command
    I->>S: Publish 0x60 (selector 1)
    S->>R: Reply pending=1
    A->>R: Poll C0F3; read C0F2 => 0x60
    I->>D: start(0x40) returns 6
    loop 5 request parameters
        A->>R: Wait C0F1 ready; write next byte
        I->>S: Poll selector 5; consume selector 0
        S-->>I: Byte; write pending clears
    end
    I->>D: receive(40 00 00 12 34 56, 6)
    D->>D: Decode 32-bit LBA and read 512-byte sector
    loop 3 header + 512 sector bytes
        D-->>I: Produce next response byte
        I->>S: Publish via selector 1
        S->>R: Reply pending=1
        A->>R: Poll C0F3; read C0F2
        I->>S: Poll selector 5 until reply pending clears
    end
```

The conceptual stages on the CPU/SPI boundary:

| Stage | Apple-visible operation | Controller-side SPI register | Meaning |
| --- | --- | --- | --- |
| Opcode | Write `$40` to `$C0F1` | Poll 5, read 0 | IOMEM command byte |
| ACK | Poll `$C0F3` and read `$C0F2` | Send 1 with `$60`; poll 5 | Opcode accepted |
| Unit | Write `$00` to `$C0F1` | Poll 5, read 0 | Logical unit zero |
| LBA | Write `$00,$12,$34,$56`, separately | Poll 5, read 0 for each | 32-bit address |
| Result header | Read `$00,$02,$00`, separately | Send 1 for each | Success + 512-byte length |
| Sector | Read 512 bytes, individually | Send 1 for each and poll 5 | Data stream |

There are **no 512-byte SPI burst frames** here. Each Apple mailbox byte is transported by its own 13-bit SPI transaction, plus status polling. Neither IOU RTL nor C0Fx registers need a 512-byte FIFO.

## 4. Apple-side reference driver (pseudocode)

The following uses conceptual `bus_read8` / `bus_write8` routines which must actually access Apple memory-mapped I/O, and bounded `wait_until_*` helpers. This is not compiled source.

```c
#define TX_DATA       0xC0F1u   // write data, read busy bit7
#define RX_DATA       0xC0F2u   // read data; acknowledges this byte
#define RX_STATUS     0xC0F3u   // bit7: byte waiting
#define PENDING       0x80u

bool send_byte(uint8_t b) {
    if (!wait_until_clear(TX_DATA, PENDING, DEADLINE)) return false;
    bus_write8(TX_DATA, b);
    return wait_until_clear(TX_DATA, PENDING, DEADLINE);
}

bool receive_byte(uint8_t *b) {
    if (!wait_until_set(RX_STATUS, PENDING, DEADLINE)) return false;
    *b = bus_read8(RX_DATA);  // clears pending in IOU
    return true;
}

bool read_sector(uint8_t unit, uint32_t lba, uint8_t out[512]) {
    uint8_t b;
    if (!send_byte(0x40)) return false;
    if (!receive_byte(&b) || b != 0x60) return false; // accept or reject

    uint8_t args[5] = {
        unit, (uint8_t)(lba >> 24), (uint8_t)(lba >> 16),
        (uint8_t)(lba >> 8), (uint8_t)lba
    };
    for (unsigned i = 0; i < 5; ++i)
        if (!send_byte(args[i])) return false;

    uint8_t status, high, low;
    if (!receive_byte(&status) ||
        !receive_byte(&high) || !receive_byte(&low)) return false;
    uint16_t size = ((uint16_t)high << 8) | low;
    if (status != 0 || size != 512) return false;

    for (unsigned i = 0; i < 512; ++i)
        if (!receive_byte(&out[i])) return false;
    return true;
}
```

A production driver should return distinct results for `0x55` invalid opcode, final error statuses, malformed header, timeout and transport resynchronisation. On timeout, **do not** immediately send a new command while bytes may still be pending.

## 5. Controller command descriptor — proposed, close to existing callbacks

The existing `iomem_controller.h` does **not** expose a descriptor or registration function. This sketch relies on the **proposed** `iomem_register_command` API and does not compile unchanged today. The storage backend calls below are conceptual, not linked functions in the repository.

```c
#define CMD_READ_LBA     0x40
#define SECTOR_BYTES     512
#define OK               0x00
#define OUT_OF_RANGE     0x02
#define IO_FAILURE       0x03

static uint8_t sector[SECTOR_BYTES];
static uint8_t result_status;
static uint16_t result_length;
static uint16_t reply_position;

// start_func returns total request size, not parameter count.
static uint16_t read_start(uint8_t opcode) {
    (void)opcode;
    result_status = IO_FAILURE;  // fail safely until backend completes
    result_length = 0;
    reply_position = 0;
    return 6;
}

static void read_receive(uint8_t *req, uint16_t len) {
    if (len != 6 || req[0] != CMD_READ_LBA) return;
    uint8_t unit = req[1];
    uint32_t lba = ((uint32_t)req[2] << 24) |
                   ((uint32_t)req[3] << 16) |
                   ((uint32_t)req[4] << 8) |
                   (uint32_t)req[5];

    if (!disk_backend_lba_valid(unit, lba)) {
        result_status = OUT_OF_RANGE;
    } else if (!disk_backend_read(unit, lba, sector, SECTOR_BYTES)) {
        result_status = IO_FAILURE;
    } else {
        result_status = OK;
        result_length = SECTOR_BYTES;
    }
    reply_position = 0;
}

// One byte per successful invocation: status, length then data.
static bool_t read_next(uint8_t *b) {
    if      (reply_position == 0) *b = result_status;
    else if (reply_position == 1) *b = (uint8_t)(result_length >> 8);
    else if (reply_position == 2) *b = (uint8_t)result_length;
    else if ((uint32_t)reply_position < (uint32_t)result_length + 3u)
        *b = sector[reply_position - 3u];
    else return FALSE;
    ++reply_position;
    return TRUE;
}

static iomem_cmd_t read_descriptor = {
    .command = CMD_READ_LBA,
    .start_func = read_start,
    .data_rx_func = read_receive,
    .data_tx_func = read_next,
    .p_next = NULL
};

void register_storage_commands(void) {
    // Call once, AFTER iomem_controller_init().
    if (!iomem_register_command(&read_descriptor))
        report_registration_error();
}
```

The first response byte is available immediately after `data_rx_func` runs.
The dispatcher calls `read_next()` once to send it, then calls again only
after the Apple has acknowledged the preceding byte. A backend which
performs asynchronous storage reads must have a distinct "device busy"
state; it must not claim success or stream stale data before completing
the sector read.

Before this sketch is actionable, export/implement registration, correct
opcode-only dispatch, validate callback sizes, add SPI queue capacity
checks and handle resets/timeouts. See the companion protocol document.

## 6. Deterministic data-integrity test

Define a test-only backing sector:

```text
sector[i] = i & 0xff, i = 0..511

Apple -> controller: 40                    ; opcode
Controller -> Apple: 60                    ; legacy acceptance ACK
Apple -> controller: 00 00 12 34 56        ; unit + LBA
Controller -> Apple: 00 02 00              ; success, 512-byte length
Controller -> Apple: 00 01 02 ... FD FE FF ; sector bytes 0..255
Controller -> Apple: 00 01 02 ... FD FE FF ; sector bytes 256..511
```

This pattern intentionally contains `0x00` and `0xFF` data bytes:
neither may be interpreted as end-of-message. Compare **all 512**
received bytes and then immediately execute a second command to
catch stale mailbox bits.

For invalid LBA, use final response `02 00 00`, verify no data bytes
are sent, and then retry a valid command.

## 7. Command registration and peripheral expansion

Opcode `0x40` and the proposed response errors are **example
reservations**, not a committed protocol-number allocation. Maintain
an opcode registry describing owner, request length, field
endianness, response length, error codes and version when implementing
real peripherals.

Other possible services can use the same byte stream:

- **Mouse:** buttons, absolute/relative movement, event dequeue.
- **RTC:** read/set time and nonvolatile setting/PRAM commands.
- **Storage:** unit enumeration, read/write LBA, status, media events.
- **SPI flash:** high-level read/write/erase services reached via
  IOMEM opcodes; the IOU's control SPI is **not** a directly exposed
  arbitrary flash bus.

Crucially, adding one of these commands must not require changing
`IOU_spi_iface.v` or assigning more SPI selectors.

## 8. Verification plan before claiming the command operational

1. Register a dummy opcode and verify exact `0x60` ACK timing.
2. Verify the handler receives exactly the bytes `40 00 00 12 34 56`.
3. Verify 32-bit LBA decode and unit selection; no host-endian dependence.
4. Return deterministic `00 02 00` plus all 512 bytes through real C0Fx accesses.
5. Exercise keyboard concurrently and confirm SPI request IDs still match.
6. Test invalid opcode, invalid unit, out-of-range block, backend I/O failure.
7. Test transaction timeout, queue saturation, reset halfway through a block.
8. Repeat back-to-back reads and compare latency to the existing
   slot hard-drive implementation; do not assert disk-class performance
   before measuring it.
9. Only then evaluate faster SPI transport options while preserving
   this Apple-visible and command-level protocol.

**Implementation status:** design/reference only. No IOMEM handler,
registration API, Apple firmware driver, storage backend adapter or
Verilog change was made with this documentation.
