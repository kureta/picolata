#include "Application.hpp"
#include "adc.hpp"
#include "modernosc.hpp"
#include "network.hpp"
#include <cstdint>
#include <iostream>
#include <memory>

#define BROADCAST_IP "255.255.255.255"
#define OSC_PORT 3333
#define ONE_SECOND 1000
#define UPDATE_RATE 120
#define ADC_PIN 0

Application::Application() : mUpdateRate(UPDATE_RATE), mAdc(ADC_PIN) {
  // Initialize network
  if (!Network::initialize()) {
    std::cout << "Failed to initialize!\n";
  }

  // Create OSC sender
  mOSC = std::make_unique<OscBuilder>(BROADCAST_IP, OSC_PORT);
}

// Main loop
void Application::update(uint32_t delta) {
  mOSC->sendOSCMessage("/delta", static_cast<float>(delta) / ONE_SECOND);
  mOSC->sendOSCMessage("/pot", ADC::getValue());
}
