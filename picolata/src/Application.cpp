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

  // Create MPU6050 sensor
  mMpu6050 = MPU6050();
}

// Main loop
void Application::update(uint32_t delta) {
  // Send time passed between loops
  mOSC->sendOSCMessage("/delta", static_cast<float>(delta) / ONE_SECOND);
  // Send pot value
  mOSC->sendOSCMessage("/pot", ADC::getValue());

  // refresh sensor value
  mMpu6050.update();

  // Read acceleration data
  Vector3D<int16_t> Accel = mMpu6050.getAccel();
  mOSC->sendOSCMessage("/accel", Accel.mX, Accel.mY, Accel.mZ);

  // Read gyroscope data
  Vector3D<int16_t> Gyro = mMpu6050.getGyro();
  mOSC->sendOSCMessage("/gyro", Gyro.mX, Gyro.mY, Gyro.mZ);

  // Read temperature data
  mOSC->sendOSCMessage("/temp", mMpu6050.getTemp());
}
