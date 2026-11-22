#-Wno-PINCONNECTEMPTY -Wno-UNUSEDSIGNAL -fno-dfg-pre-inline
export VERILATOR_ROOT="$HOME/Tools/verilator/"
export PROJECT_DEFINES="-D_DEBUG_GPIO -DEXTERNAL_CPU -DVERILATOR_RUN -DEXTERNAL_MEMORY"

verilator  --cc --build -Wall -Wno-UNUSEDPARAM --pins-inout-enables $PROJECT_DEFINES -I$VERILATOR_ROOT/include $1
