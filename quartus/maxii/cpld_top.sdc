# EXPERIMENTAL clocks ONLY; NOT physical/timing sign-off.
# Matches the previously simulated 1 MHz PHI_0 / 125 kHz SPI SCK point.
# The actual board clocks, I/O delays, setup/hold and CDC policy MUST be
# qualified against the schematic and connected W65C02S/STM32/SRAM/Flash.
create_clock -name PHI_0 -period 1000.000 [get_ports {PHI_0}]
create_clock -name SPI_CLK -period 8000.000 [get_ports {spi_clk}]
# Intentionally no set_clock_groups -asynchronous: the SPI/PHI_0 mailbox
# transfers are not validated by simply hiding inter-clock paths.
# No fabricated pin locations, false paths, input or output delay values.
