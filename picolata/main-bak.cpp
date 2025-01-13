#include "adc.hpp"
#include "device/usbd.h"
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

#include "hardware/gpio.h"
#include "pico/binary_info.h"

#include "bsp/board.h"
#include "tusb.h"

// Variable that holds the current position in the sequence.
uint32_t note_pos = 0;

// Store example melody as an array of note values
uint8_t note_sequence[] = {
    74, 78, 81, 86,  90, 93, 98, 102, 57, 61,  66, 69, 73, 78, 81, 85,
    88, 92, 97, 100, 97, 92, 88, 85,  81, 78,  74, 69, 66, 62, 57, 62,
    66, 69, 74, 78,  81, 86, 90, 93,  97, 102, 97, 93, 90, 85, 81, 78,
    73, 68, 64, 61,  56, 61, 64, 68,  74, 78,  81, 86, 90, 93, 98, 102};

void midi_task(void) {
  static uint32_t start_ms = 0;
  uint8_t msg[3];

  // send note every 1000 ms
  if (board_millis() - start_ms < 286)
    return; // not enough time
  start_ms += 286;

  // Previous positions in the note sequence.
  int previous = note_pos - 1;

  // If we currently are at position 0, set the
  // previous position to the last note in the sequence.
  if (previous < 0)
    previous = sizeof(note_sequence) - 1;

  // Send Note On for current position at full velocity (127) on channel 1.
  msg[0] = 0x90;                    // Note On - Channel 1
  msg[1] = note_sequence[note_pos]; // Note Number
  msg[2] = 127;                     // Velocity
  tud_midi_n_stream_write(0, 0, msg, 3);

  // Send Note Off for previous note.
  msg[0] = 0x80;                    // Note Off - Channel 1
  msg[1] = note_sequence[previous]; // Note Number
  msg[2] = 0;                       // Velocity
  tud_midi_n_stream_write(0, 0, msg, 3);

  // Increment position
  note_pos++;

  // If we are at the end of the sequence, start over.
  if (note_pos >= sizeof(note_sequence))
    note_pos = 0;
}

#define SYSTEM_WAIT 1000
#define BUFFER_SIZE 1024

int main() {
  board_init();

  tusb_init();
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

    Buffer.clear();
    Buffer = OscBuilder::makeOscPacket("/gyro", gyro_dps[0], gyro_dps[1],
                                       gyro_dps[2]);
    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());

    Buffer.clear();
    Buffer = OscBuilder::makeOscPacket("/temp", temp / 340.0F + 36.53F);
    Network::send(Buffer.data(), DestAddr, Port, Buffer.size());

    tud_task();
    midi_task();

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
