#!/bin/env sh

PROBE="/usr/share/openocd/scripts/interface/cmsis-dap.cfg"
TARGET="/usr/share/openocd/scripts/target/rp2040.cfg"
SCRIPTS="/usr/share/openocd/scripts/"

openocd -f "$PROBE" -c 'set USE_CORE 0' -f "$TARGET" -c "adapter speed 5000" -s "$SCRIPTS"
