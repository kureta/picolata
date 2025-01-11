#include "adc.hpp"
#include "modernosc.hpp"
#include "mpu6050.hpp"
#include "network.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <lwip/def.h>
#include <lwip/inet.h>
#include <pico/stdlib.h> // IWYU pragma: keep
#include <pico/time.h>
#include <string>
#include <vector>

#define SYSTEM_WAIT 1000
#define BUFFER_SIZE 1024

int main() {
  // Initialize stdio for printf
  stdio_init_all();
  // Have to wait a bit to be able to use printf
  sleep_ms(SYSTEM_WAIT);

  // Initialize app
  if (!Network::initialize()) {
    std::cout << "Failed to initialize!\n";
    return -1;
  }

  // OSC parameters
  std::string DestAddr = "255.255.255.255";
  const unsigned int Port = 3333;
  // Empty buffer to put the message into

  // Initialize adc
  ADC::initialize();

  // Initialize MPU6050
  mpu6050_initialize();
  int16_t accel[3], gyro[3], temp;

  // App start
  std::cout << "Entering main loop.\n";
  // Set the desired loop frequency (n times per second)
  const int Ops = 120;               // for example, 10Hz
  const int IntervalMs = 1000 / Ops; // Convert frequency to milliseconds
  while (true) {
    // Record the start time
    uint32_t StartTime = to_ms_since_boot(get_absolute_time());

    // Your main loop code goes here
    std::vector<char> Buffer =
        OscBuilder::makeOscPacket("/pot", ADC::getValue());
    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());

    // MPU6050
    mpu6050_read_raw(accel, gyro, &temp);
    // Convert raw accelerometer values to g
    float accel_g[3];
    accel_g[0] = accel[0] / ACCEL_SCALE_FACTOR;
    accel_g[1] = accel[1] / ACCEL_SCALE_FACTOR;
    accel_g[2] = accel[2] / ACCEL_SCALE_FACTOR;

    // Convert raw gyroscope values to degrees per second
    float gyro_dps[3];
    gyro_dps[0] = gyro[0] / GYRO_SCALE_FACTOR;
    gyro_dps[1] = gyro[1] / GYRO_SCALE_FACTOR;
    gyro_dps[2] = gyro[2] / GYRO_SCALE_FACTOR;

    Buffer.clear();
    Buffer =
        OscBuilder::makeOscPacket("/accel", accel_g[0], accel_g[1], accel_g[2]);
    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());

    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());
    Buffer.clear();
    Buffer = OscBuilder::makeOscPacket("/gyro", gyro_dps[0], gyro_dps[1],
                                       gyro_dps[2]);
    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());

    Buffer.clear();
    Buffer = OscBuilder::makeOscPacket("/temp", temp / 340.0F + 36.53F);
    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());

    // Calculate elapsed time
    uint32_t ElapsedTime = to_ms_since_boot(get_absolute_time()) - StartTime;

    // Check if the loop overran
    if (ElapsedTime > IntervalMs) {
      // std::cout << "overrun\n";
    } else {
      // Sleep until the next loop cycle
      sleep_ms(IntervalMs - ElapsedTime);
    }
  }

  std::cout << "Exiting\n";
  Network::deinitialize();
  sleep_ms(SYSTEM_WAIT);

  return 0;
}
