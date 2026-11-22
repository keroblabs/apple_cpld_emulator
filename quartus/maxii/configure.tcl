# Configures a COPY of the tracked project inside one isolated remote build.
# Usage: quartus_sh -t configure.tcl EPM240T100C5 compact
package require ::quartus::project

if {[info exists quartus(args)]} {
    set args $quartus(args)
} else {
    set args $argv
}
if {[llength $args] != 2} {
    error "Expected PART {compact|full}"
}
set part [lindex $args 0]
set variant [lindex $args 1]
if {$part ni {EPM240T100C5 EPM570T100C5 EPM570T144C5}} {
    error "Unsupported MAX II part: $part"
}
if {$variant ni {compact full}} {
    error "Unsupported address variant: $variant"
}

project_open cpld_top
set_global_assignment -name DEVICE $part
if {$variant eq "full"} {
    set_global_assignment -name VERILOG_MACRO "FULL_MMU_ADDR=1"
}
export_assignments
project_close
puts "Configured MAX II cpld_top for $part ($variant)"
