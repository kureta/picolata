#include "adc.hpp"
#include <array>
#include <hardware/adc.h>
#include <pico/cyw43_arch.h>

const std::array<int, 3> ADC::kAdcToGpio = {26, 27, 28};

ADC::ADC(int adcIndex) : mAdcIndex(adcIndex) {
  // Initialize ADC
  adc_init();

  // Make sure GPIO is high-impedance, no pullups etc
  adc_gpio_init(kAdcToGpio.at(mAdcIndex));
  // Select ADC input `mAdcIndex`
  adc_select_input(mAdcIndex);
}

float ADC::getValue() {
  uint16_t Result = adc_read();
  float Value = static_cast<float>(Result) / ADC_RESOLUTION;

  return Value;
}
