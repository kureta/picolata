#include "adc.hpp"
#include "network.hpp"
#include "tinyosc.hpp"
#include <pico/stdlib.h> // IWYU pragma: keep
#include <pico/time.h>

int main() {
  // Initialize stdio for printf
  stdio_init_all();
  // Have to wait a bit to be able to use printf
  sleep_ms(1000);

  // Initialize app
  if (!Network::initialize()) {
    printf("Failed to initialize!");
    return -1;
  }

  // OSC parameters
  char dest_addr[] = "255.255.255.255";
  const unsigned int port = 3333;
  // Empty buffer to put the message into
  char buffer[1024];

  // Initialize osc
  TinyOSC osc = TinyOSC();

  // Initialize adc
  ADC adc = ADC();

  // App start
  printf("Entering main loop.");
  while (true) {
    osc.writeMessage(buffer, sizeof(buffer), "/ping", "f", adc.get_value());
    Network::send(buffer, dest_addr, port);
    sleep_ms(500);
  }

  printf("Exiting");
  Network::deinitialize();
  sleep_ms(1000);

  return 0;
}
