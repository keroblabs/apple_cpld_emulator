#include <cstdint>
#include <cstdio>
#include "VIOU_spi_iface.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

static unsigned checks;
static unsigned failures;

static void check(bool ok, const char *message)
{
    ++checks;
    if (!ok) {
        ++failures;
        std::printf("FAIL: %s\n", message);
    }
}

class SpiHarness {
private:
    VerilatedVcdC *trace_;
    uint64_t time_;

public:
    VIOU_spi_iface dut;

    SpiHarness() : trace_(nullptr), time_(0)
    {
    }

    void attach_trace(VerilatedVcdC *trace)
    {
        trace_ = trace;
    }

    void evaluate()
    {
        dut.eval();
        if (trace_) {
            trace_->dump(time_++);
        }
    }

    void initialize()
    {
        dut.spi_cs = 1;
        dut.spi_clk = 0;
        dut.spi_mosi = 0;
        dut.reg_wr_data = 0;
        dut.reg_rd_pending_state = 0;
        dut.reg_wr_pending_state = 0;
        dut.reg_kbd_pending_state = 0;
        evaluate();
        preload();
    }

    void preload()
    {
        dut.spi_cs = 1;
        dut.spi_clk = 0;
        dut.spi_mosi = 0;
        evaluate();
        dut.spi_clk = 1;
        evaluate();
        dut.spi_clk = 0;
        evaluate();
    }

    uint16_t transfer(uint8_t selector, uint8_t payload, bool trailing_clock = true)
    {
        const uint16_t word = (static_cast<uint16_t>(selector & 0x1F) << 8) | payload;
        uint16_t received = 0;

        dut.spi_cs = 0;
        dut.spi_clk = 0;
        evaluate();
        for (int bit = 12; bit >= 0; --bit) {
            dut.spi_mosi = (word >> bit) & 1;
            dut.spi_clk = 0;
            evaluate();
            received = static_cast<uint16_t>((received << 1) | (dut.spi_miso & 1));
            dut.spi_clk = 1;
            evaluate();
        }

        if (trailing_clock) {
            clock_bit(0);
        }
        return received;
    }

    void clock_bit(uint8_t mosi)
    {
        dut.spi_clk = 0;
        dut.spi_mosi = mosi & 1;
        evaluate();
        dut.spi_clk = 1;
        evaluate();
    }

    void close()
    {
        dut.spi_cs = 1;
        dut.spi_clk = 0;
        evaluate();
    }

    void expect_idle_commands(const char *test)
    {
        check(dut.reg_wr_pending_clr == 1, test);
        check(dut.reg_rd_pending_set == 1, test);
        check(dut.reg_kbd_pending_set == 1, test);
    }
};

static uint8_t status_prefix(const SpiHarness &spi)
{
    return static_cast<uint8_t>(((spi.dut.reg_kbd_pending_state & 1) << 2) |
                                ((spi.dut.reg_rd_pending_state & 1) << 1) |
                                (spi.dut.reg_wr_pending_state & 1));
}

static void expect_transfer(SpiHarness &spi, uint8_t selector, uint8_t payload,
                            uint8_t expected_data, const char *test)
{
    spi.preload();
    const uint8_t status = status_prefix(spi);
    const uint16_t received = spi.transfer(selector, payload, false);
    const uint16_t expected = (static_cast<uint16_t>(status) << 8) | expected_data;
    if (received != expected) {
        std::printf("  selector=%u received=$%04X expected=$%04X\n",
                    selector, received, expected);
    }
    check(received == expected, test);
}

static void test_valid_selectors_and_status(SpiHarness &spi)
{
    spi.dut.reg_wr_data = 0xA5;
    spi.dut.reg_kbd_pending_state = 1;
    spi.dut.reg_rd_pending_state = 0;
    spi.dut.reg_wr_pending_state = 1;

    expect_transfer(spi, 0, 0, 0xA5, "selector 0 returns all five status bits and mailbox data");
    check(spi.dut.reg_wr_pending_clr == 0, "selector 0 pulses only the write-pending clear");
    check(spi.dut.reg_rd_pending_set == 1 && spi.dut.reg_kbd_pending_set == 1,
          "selector 0 leaves the other pending controls inactive");
    spi.clock_bit(0);
    spi.expect_idle_commands("the trailing clock releases the selector 0 side effect");
    spi.close();

    expect_transfer(spi, 1, 0x96, 0xFF, "selector 1 returns status plus the ignored response byte");
    check(spi.dut.reg_rd_data == 0x96, "selector 1 captures all eight payload bits");
    check(spi.dut.reg_rd_pending_set == 0, "selector 1 pulses the read-pending set");
    spi.clock_bit(0);
    spi.close();

    expect_transfer(spi, 2, 0xD3, 0xFF, "selector 2 status and keyboard transaction");
    check(spi.dut.reg_kbd_data == 0x53, "selector 2 captures the seven-bit keyboard value");
    check(spi.dut.reg_kbd_pending_set == 0, "selector 2 pulses the keyboard-pending set");
    spi.clock_bit(0);
    spi.close();

    spi.dut.reg_wr_data = 0x00;
    spi.dut.reg_kbd_pending_state = 0;
    spi.dut.reg_rd_pending_state = 1;
    spi.dut.reg_wr_pending_state = 0;
    expect_transfer(spi, 5, 0xFF, 0xFF, "selector 5 returns status and ignores response payload");
    spi.expect_idle_commands("selector 5 has no mailbox side effects");
    spi.clock_bit(0);
    spi.close();
}

static void test_all_payload_bits(SpiHarness &spi)
{
    for (unsigned bit = 0; bit < 8; ++bit) {
        const uint8_t payload = static_cast<uint8_t>(1U << bit);
        spi.preload();
        const uint16_t received = spi.transfer(1, payload, false);
        check(spi.dut.reg_rd_data == payload, "selector 1 preserves each payload bit position");
        check((received & 0x00FF) == 0xFF, "selector 1 response payload remains unspecified high");
        spi.clock_bit(0);
        spi.close();
    }
}

static void test_full_selector_decode(SpiHarness &spi)
{
    const uint8_t unsupported[] = {8, 9, 10, 17};
    for (uint8_t selector : unsupported) {
        spi.preload();
        expect_transfer(spi, selector, 0x6D, 0xFF,
                        "upper selector bits prevent aliases of selectors 0, 1 and 2");
        spi.expect_idle_commands("unsupported five-bit selector has no side effect");
        spi.clock_bit(0);
        spi.close();
    }
}

static void test_abort_and_extra_clocks(SpiHarness &spi)
{
    spi.preload();
    spi.dut.spi_cs = 0;
    spi.dut.spi_clk = 0;
    spi.evaluate();
    for (int bit = 12; bit >= 7; --bit) {
        spi.dut.spi_mosi = (0x0123 >> bit) & 1;
        spi.dut.spi_clk = 1;
        spi.evaluate();
        spi.dut.spi_clk = 0;
        spi.evaluate();
    }
    spi.dut.spi_cs = 1;
    spi.evaluate();
    spi.expect_idle_commands("CS abort immediately suppresses pending side effects");
    spi.preload();

    spi.dut.reg_wr_data = 0xA5;
    spi.dut.reg_wr_pending_state = 1;
    spi.dut.reg_kbd_pending_state = 0;
    spi.dut.reg_rd_pending_state = 0;
    expect_transfer(spi, 0, 0x3C, 0xA5,
                    "first complete frame after abort starts from the preload state");
    check(spi.dut.reg_wr_pending_clr == 0, "valid frame after abort executes once");
    spi.clock_bit(0); // Required trailing clock moves the count to its terminal state.
    spi.expect_idle_commands("trailing edge ends the frame");

    for (unsigned edge = 0; edge < 15; ++edge) {
        spi.clock_bit(0);
    }
    spi.expect_idle_commands("extra clocks cannot wrap the bit counter into another command");
    spi.close();
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);
    SpiHarness spi;
    VerilatedVcdC waveform;
    spi.dut.trace(&waveform, 99);
    const char *waveform_path = argc > 1 ? argv[1] : "spi_iface_waveform.vcd";
    waveform.open(waveform_path);
    spi.attach_trace(&waveform);
    spi.initialize();
    test_valid_selectors_and_status(spi);
    test_all_payload_bits(spi);
    test_full_selector_decode(spi);
    test_abort_and_extra_clocks(spi);
    waveform.close();

    std::printf("IOU SPI: %u checks, %u failures\n", checks, failures);
    return failures != 0;
}
