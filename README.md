# Picolata: A chocolaty library for Raspberry Pi Pico/Pico W

## Debugging

- install `arm-none-eabi-gdb` and `openocd`
- run `openocd -f /usr/share/openocd/scripts/interface/cmsis-dap.cfg
-f /usr/share/openocd/scripts/target/rp2040.cfg -c "adapter speed 5000"
-s /usr/share/openocd/scripts/`
- in another window, run `gdb out/Debug/picolata.elf`
- in `gdb`, run these commands:

  ```gdb
  target extended-remote localhost:3333
  load
  monitor reset
  b main
  c
  ```

  Using `--eval-command load` for `nvim-dap` breaks it for some reason.
  You have to manually type `load` to `dap` `repl` for breakpoints to work.

  Good luck!

## TODOs

- [ ] Decide on one typing convention (`u_int8_t`, `uint8_t`, `unsigned short int`, etc.)
- [ ] Fix sensor data retrieval and scaling
