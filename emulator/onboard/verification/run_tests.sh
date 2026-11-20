#!/usr/bin/env bash
# CPLD-ROM-0003 — build and run the rom_backing unit tests (standalone).
set -euo pipefail
cd "$(dirname "$0")"

CC="${CC:-cc}"
OUT=test_rom_backing

"$CC" -std=c11 -Wall -Wextra -Werror -O2 \
    -I.. \
    test_rom_backing.c ../rom_backing.c \
    -o "$OUT"

./"$OUT"
