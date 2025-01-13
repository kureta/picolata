#include "Application.hpp"
#include <cstdint>
#include <iostream>
#include <pico/stdlib.h> // IWYU pragma: keep
#include <sys/_types.h>

#define SYSTEM_WAIT 1000
#define BUFFER_SIZE 1024

int main() {
  // Initialize stdio for printf
  stdio_init_all();
  // Have to wait a bit to be able to use printf
  sleep_ms(SYSTEM_WAIT);

  // App Initialize
  Application App = Application();

  std::cout << "Entering main loop.\n";
  // Set the desired loop frequency (n times per second)
  const unsigned int Ops = App.getUpdateRate(); // for example, 10Hz
  const unsigned int IntervalMs =
      1000 / Ops; // Convert frequency to milliseconds

  uint32_t ElapsedTime = 0;
  uint32_t StartTime = 0;
  uint32_t Delta = 0;
  while (true) {
    // Record the start time
    StartTime = to_ms_since_boot(get_absolute_time());

    // Your main loop code goes here
    App.update(Delta);

    // Calculate elapsed time
    ElapsedTime = to_ms_since_boot(get_absolute_time()) - StartTime;

    // Check if the loop overran
    if (ElapsedTime > IntervalMs) {
      // std::cout << "overrun\n";
    } else {
      // Sleep until the next loop cycle
      sleep_ms(IntervalMs - ElapsedTime);
    }

    Delta = to_ms_since_boot(get_absolute_time()) - StartTime;
  }

  std::cout << "Exiting\n";

  sleep_ms(SYSTEM_WAIT);

  return 0;
}
