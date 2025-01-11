#include "adc.hpp"
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>

#define LED_PIN 26

void ADC::initialize() {
  adc_init();
  // Make sure GPIO is high-impedance, no pullups etc
  adc_gpio_init(LED_PIN);
  // Select ADC input 0 (GPIO26)
  adc_select_input(0);
}

float ADC::getValue() {
  uint16_t Result = adc_read();
  float Value = static_cast<float>(Result) / ADC_RESOLUTION;

  return Value;
}
