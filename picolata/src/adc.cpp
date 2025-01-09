#include "adc.hpp"
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>

ADC::ADC() {
  adc_init();
  // Make sure GPIO is high-impedance, no pullups etc
  adc_gpio_init(26);
  // Select ADC input 0 (GPIO26)
  adc_select_input(0);
}

float ADC::get_value() {
  uint16_t result = adc_read();
  float value = result * conversion_factor;

  return value;
}
