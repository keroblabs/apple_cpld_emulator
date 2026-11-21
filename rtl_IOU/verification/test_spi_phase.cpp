#include <cstdint>
#include <cstdio>

#include "VIOU_spi_phase_tb.h"
#include "verilated.h"
#include "verilated_vcd_c.h"

static constexpr uint64_t PHI0_PERIOD_NS = 1000;
static constexpr uint64_t PHI0_HALF_NS = PHI0_PERIOD_NS / 2;
static constexpr uint64_t SPI_HALF_NS = 4 * PHI0_PERIOD_NS;
static constexpr uint64_t SPI_PERIOD_NS = 2 * SPI_HALF_NS;
static constexpr unsigned PHASE_COUNT = PHI0_PERIOD_NS;
static constexpr unsigned TRACE_PHASE_NS = PHI0_HALF_NS + 1;

static unsigned checks;
static unsigned failures;

static void check(bool ok, const char *message, unsigned phase)
{
    ++checks;
    if (!ok) {
        ++failures;
        std::printf("FAIL phase=%uns: %s\n", phase, message);
    }
}

class PhaseHarness {
private:
    VerilatedVcdC *trace_;
    bool trace_enabled_;
    uint64_t now_ns_;
    uint64_t next_phi_edge_ns_;

    void evaluate()
    {
        dut.eval();
        if (trace_ && trace_enabled_) {
            trace_->dump(now_ns_);
        }
    }

public:
    VIOU_spi_phase_tb dut;

    PhaseHarness()
        : trace_(nullptr), trace_enabled_(false), now_ns_(0),
          next_phi_edge_ns_(PHI0_HALF_NS)
    {
    }

    uint64_t now_ns() const { return now_ns_; }

    void attach_trace(VerilatedVcdC *trace)
    {
        trace_ = trace;
    }

    template <typename Drive>
    void at(uint64_t time_ns, Drive drive)
    {
        while (next_phi_edge_ns_ < time_ns) {
            now_ns_ = next_phi_edge_ns_;
            dut.PHI_0 = !dut.PHI_0;
            next_phi_edge_ns_ += PHI0_HALF_NS;
            evaluate();
        }

        now_ns_ = time_ns;
        const bool phi_edge = next_phi_edge_ns_ == time_ns;
        if (phi_edge) {
            dut.PHI_0 = !dut.PHI_0;
            next_phi_edge_ns_ += PHI0_HALF_NS;
        }
        drive(dut);
        evaluate();
    }

    void at(uint64_t time_ns)
    {
        at(time_ns, [](VIOU_spi_phase_tb &) {});
    }

    void wait_rising_edges(unsigned count)
    {
        while (count != 0) {
            at(next_phi_edge_ns_);
            if (dut.PHI_0) {
                --count;
            }
        }
    }

    void initialize(VerilatedVcdC *trace)
    {
        trace_ = trace;
        dut.RESET = 0;
        dut.PHI_0 = 0;
        dut.CPU_RW = 1;
        dut.ADDR = 0;
        dut.C01X_N = 1;
        dut.C0FX_N = 1;
        dut.dbus_in = 0;
        dut.status_AN3 = 0;
        dut.DHIRES_state_n = 0;
        dut.spi_cs = 1;
        dut.spi_clk = 0;
        dut.spi_mosi = 0;
        evaluate();
        at(3 * PHI0_PERIOD_NS);
        at(3 * PHI0_PERIOD_NS, [](VIOU_spi_phase_tb &model) {
            model.RESET = 1;
        });
        wait_rising_edges(2);
    }

    void trace_phase(bool enabled)
    {
        trace_enabled_ = enabled;
    }

    void align_spi_phase(unsigned phase_ns)
    {
        uint64_t target = now_ns_;
        const unsigned current_phase = static_cast<unsigned>(target % PHI0_PERIOD_NS);
        const unsigned delta = (phase_ns + PHI0_PERIOD_NS - current_phase) % PHI0_PERIOD_NS;
        if (delta != 0) {
            at(target + delta);
        }
    }

    void cpu_write_mailbox()
    {
        at(now_ns_ + 1, [](VIOU_spi_phase_tb &model) {
            model.C0FX_N = 0;
            model.ADDR = 1;
            model.CPU_RW = 0;
            model.dbus_in = 0xA5;
        });
        wait_rising_edges(3);
        at(now_ns_ + 1, [](VIOU_spi_phase_tb &model) {
            model.C0FX_N = 1;
            model.CPU_RW = 1;
        });
        wait_rising_edges(2);
    }

    void cpu_ack_read_pending()
    {
        at(now_ns_ + 1, [](VIOU_spi_phase_tb &model) {
            model.C0FX_N = 0;
            model.ADDR = 2;
            model.CPU_RW = 1;
        });
        wait_rising_edges(3);
        at(now_ns_ + 1, [](VIOU_spi_phase_tb &model) {
            model.C0FX_N = 1;
        });
        wait_rising_edges(2);
    }

    void cpu_ack_keyboard_pending()
    {
        at(now_ns_ + 1, [](VIOU_spi_phase_tb &model) {
            model.C01X_N = 0;
            model.CPU_RW = 0;
        });
        wait_rising_edges(3);
        at(now_ns_ + 1, [](VIOU_spi_phase_tb &model) {
            model.C01X_N = 1;
            model.CPU_RW = 1;
        });
        wait_rising_edges(2);
    }

    void transfer(uint8_t selector, unsigned phase_ns)
    {
        align_spi_phase(phase_ns);
        at(now_ns_ + SPI_HALF_NS, [](VIOU_spi_phase_tb &model) {
            model.spi_clk = 1;
        });
        at(now_ns_ + SPI_HALF_NS, [selector](VIOU_spi_phase_tb &model) {
            model.spi_clk = 0;
            model.spi_cs = 0;
            model.spi_mosi = (selector >> 4) & 1;
        });

        uint64_t request_assert_ns = 0;
        for (int bit = 12; bit >= 0; --bit) {
            const uint16_t word = static_cast<uint16_t>(selector & 0x1f) << 8;
            at(now_ns_ + SPI_HALF_NS, [](VIOU_spi_phase_tb &model) {
                model.spi_clk = 1;
            });
            if (bit == 0) {
                request_assert_ns = now_ns_;
                check_request(selector, phase_ns, true);
            }
            const uint8_t next_mosi = bit > 0
                ? static_cast<uint8_t>((word >> (bit - 1)) & 1)
                : 0;
            at(now_ns_ + SPI_HALF_NS, [next_mosi](VIOU_spi_phase_tb &model) {
                model.spi_clk = 0;
                model.spi_mosi = next_mosi;
            });
        }

        // The legacy trailing edge ends the one-SCK-period pending request.
        at(now_ns_ + SPI_HALF_NS, [](VIOU_spi_phase_tb &model) {
            model.spi_clk = 1;
        });
        check(now_ns_ - request_assert_ns == SPI_PERIOD_NS,
              "pending request hold is one complete 8 us SCK period", phase_ns);
        check_request(selector, phase_ns, false);
        at(now_ns_ + SPI_HALF_NS, [](VIOU_spi_phase_tb &model) {
            model.spi_clk = 0;
            model.spi_cs = 1;
        });
        check(dut.spi_clk == 0 && dut.spi_cs == 1,
              "frame closes with SCK low and CS high", phase_ns);
    }

    void check_request(uint8_t selector, unsigned phase_ns, bool expected_active)
    {
        const bool wr_active = dut.wr_pending_clear_req == 0;
        const bool rd_active = dut.rd_pending_set_req == 0;
        const bool kbd_active = dut.kbd_pending_set_req == 0;
        const bool actual = selector == 0 ? wr_active
                           : selector == 1 ? rd_active
                           : kbd_active;
        const bool others_idle = selector == 0 ? (!rd_active && !kbd_active)
                                 : selector == 1 ? (!wr_active && !kbd_active)
                                 : (!wr_active && !rd_active);
        check(actual == expected_active,
              expected_active ? "selected pending request is asserted for the frame"
                              : "trailing edge releases the pending request",
              phase_ns);
        check(others_idle, "unselected pending requests remain inactive", phase_ns);
    }
};

static void test_iou_an3_status(PhaseHarness &h)
{
    h.at(h.now_ns() + 1, [](VIOU_spi_phase_tb &model) {
        model.CPU_RW = 1;
        model.C01X_N = 1;
        model.C0FX_N = 1;
        model.status_AN3 = 0;
        model.DHIRES_state_n = 0;
    });
    h.wait_rising_edges(1);
    check(h.dut.io_data_en == 0 && (h.dut.io_data_out & 0x80) == 0,
          "IOU status read returns a low internal AN3 value", 0);

    h.at(h.now_ns() + 1, [](VIOU_spi_phase_tb &model) {
        model.status_AN3 = 1;
    });
    h.wait_rising_edges(1);
    check(h.dut.io_data_en == 0 && (h.dut.io_data_out & 0x80) != 0,
          "IOU status read still mirrors internal AN3 high", 0);

    h.at(h.now_ns() + 1, [](VIOU_spi_phase_tb &model) {
        model.DHIRES_state_n = 1;
    });
}

static void test_one_phase(PhaseHarness &h, unsigned phase_ns)
{
    h.cpu_write_mailbox();
    check(h.dut.wr_pending_state == 1,
          "PHI_0 captures the Apple-to-SPI mailbox request", phase_ns);

    h.transfer(0, phase_ns);
    check(h.dut.wr_pending_state == 0,
          "SPI selector 0 clears write-pending at every phase", phase_ns);

    h.transfer(1, phase_ns);
    check(h.dut.rd_pending_state == 1,
          "SPI selector 1 sets read-pending at every phase", phase_ns);
    h.cpu_ack_read_pending();
    check(h.dut.rd_pending_state == 0,
          "PHI_0 captures the Apple read acknowledgement", phase_ns);

    h.transfer(2, phase_ns);
    check(h.dut.kbd_pending_state == 1,
          "SPI selector 2 sets keyboard-pending at every phase", phase_ns);
    h.cpu_ack_keyboard_pending();
    check(h.dut.kbd_pending_state == 0,
          "PHI_0 captures the Apple keyboard acknowledgement", phase_ns);
}

int main(int argc, char **argv)
{
    Verilated::commandArgs(argc, argv);
    Verilated::traceEverOn(true);

    PhaseHarness h;
    VerilatedVcdC trace;
    const char *waveform_path = argc > 1 ? argv[1] : nullptr;
    if (waveform_path) {
        h.dut.trace(&trace, 99);
        trace.open(waveform_path);
        h.attach_trace(&trace);
    }
    h.initialize(waveform_path ? &trace : nullptr);
    test_iou_an3_status(h);

    for (unsigned phase = 0; phase < PHASE_COUNT; ++phase) {
        h.trace_phase(waveform_path && phase == TRACE_PHASE_NS);
        test_one_phase(h, phase);
    }

    if (waveform_path) {
        trace.close();
    }

    std::printf("IOU SPI/PHI_0 phase: %u checks, %u failures; %u one-nanosecond phase offsets; PHI_0=1MHz, SCK=125kHz.\n",
                checks, failures, PHASE_COUNT);
    return failures != 0;
}
