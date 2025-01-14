#include "mpu6050.hpp"
#include <array>
#include <bit>
#include <cstdint>
#include <format>
#include <iostream>

MPU6050::MPU6050() {
  // Initialize I2C
  i2c_init(I2C_PORT, BAUD_RATE);
  gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(SDA_PIN);
  gpio_pull_up(SCL_PIN);

  // Reset and configure MPU6050
  mpu6050Reset();
  mpu6050Configure();

  uint8_t WhoAmI = 0;
  uint8_t Reg = WHO_AM_I_REG;
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &Reg, 1, true);
  i2c_read_blocking(I2C_PORT, MPU6050_ADDR, &WhoAmI, 1, false);
  std::cout << "MPU6050 WHO_AM_I: " << std::format("0x{:x}", WhoAmI);

  if (WhoAmI != MPU6050_ID) {
    std::cout << "MPU6050 not found!";
  }
}

void MPU6050::update() { mpu6050ReadRaw(); }

// FIX: all scaling calculations and/or buffer read from sensor are/is wrong!
[[nodiscard]] Vector3D<int16_t> MPU6050::getAccel() const {
  return mSensorData.mAccel / ACCEL_SCALE_FACTOR;
}

[[nodiscard]] Vector3D<int16_t> MPU6050::getGyro() const {
  return mSensorData.mGyro / GYRO_SCALE_FACTOR;
}

[[nodiscard]] float MPU6050::getTemp() const {
  uint16_t Temp = mSensorData.mTemp;
  return (static_cast<float>(Temp) / TEMP_SCALE_FACTOR) + TEMP_OFFSET;
}

void MPU6050::mpu6050Configure() {
  // Set accelerometer range
  std::array<uint8_t, 2> AccelConfig = {REG_ACCEL_CONFIG, ACCEL_CONFIG_VALUE};
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, AccelConfig.data(), 2, false);

  // Set gyroscope range
  std::array<uint8_t, 2> GyroConfig = {REG_GYRO_CONFIG, GYRO_CONFIG_VALUE};
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, GyroConfig.data(), 2, false);

  // Set sample rate
  std::array<uint8_t, 2> SampleRate = {REG_SMPLRT_DIV, SAMPLE_RATE_DIV};
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, SampleRate.data(), 2, false);
}

void MPU6050::mpu6050Reset() {
  std::array<uint8_t, 2> Reset = {REG_PWR_MGMT_1, PWR_RESET};
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, Reset.data(), 2, false);
  sleep_ms(SENSOR_WAIT);
  std::array<uint8_t, 2> Wake = {REG_PWR_MGMT_1, PWR_WAKE};
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, Wake.data(), 2, false);
  sleep_ms(SENSOR_WAIT);
}

void MPU6050::mpu6050ReadRaw() {
  std::array<uint8_t, SENSOR_DATA_LENGTH> Buffer{};
  uint8_t Reg = REG_ACCEL_XOUT_H;
  i2c_write_blocking(I2C_PORT, MPU6050_ADDR, &Reg, 1, true);
  i2c_read_blocking(I2C_PORT, MPU6050_ADDR, Buffer.data(), SENSOR_DATA_LENGTH,
                    false);

  mSensorData =
      std::bit_cast<MPU6050Data, std::array<uint8_t, SENSOR_DATA_LENGTH>>(
          Buffer);
};
