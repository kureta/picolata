#ifndef INCLUDE_MPU6050_HPP_
#define INCLUDE_MPU6050_HPP_

#include "hardware/i2c.h"
#include <cmath>
#include <pico/stdlib.h> // IWYU pragma: keep

// I2C defines
#define I2C_PORT i2c0
#define MPU6050_ADDR 0x68
#define MPU6050_ID 0x68

// MPU6050 register addresses
#define REG_PWR_MGMT_1 0x6B
#define REG_ACCEL_XOUT_H 0x3B
#define REG_GYRO_CONFIG 0x1B
#define REG_ACCEL_CONFIG 0x1C
#define REG_SMPLRT_DIV 0x19
#define WHO_AM_I_REG 0x75

// Sensitivity scale factors for different ranges
#define ACCEL_SCALE_FACTOR_2G 16384.0 // for ±2g
#define ACCEL_SCALE_FACTOR_4G 8192.0  // for ±4g
#define ACCEL_SCALE_FACTOR_8G 4096.0  // for ±8g
#define ACCEL_SCALE_FACTOR_16G 2048.0 // for ±16g

#define GYRO_SCALE_FACTOR_250DPS 131.0 // for ±250 degrees per second
#define GYRO_SCALE_FACTOR_500DPS 65.5  // for ±500 degrees per second
#define GYRO_SCALE_FACTOR_1000DPS 32.8 // for ±1000 degrees per second
#define GYRO_SCALE_FACTOR_2000DPS 16.4 // for ±2000 degrees per second

#define TEMP_SCALE_FACTOR 340.0F
#define TEMP_OFFSET 36.53F

// Select the desired scale factor
#define ACCEL_SCALE_FACTOR                                                     \
  ACCEL_SCALE_FACTOR_4G // Change this to the desired accelerometer range
#define GYRO_SCALE_FACTOR                                                      \
  GYRO_SCALE_FACTOR_250DPS // Change this to the desired gyroscope range

// Corresponding configuration values
#define ACCEL_CONFIG_VALUE 0x08 // for ±4g
#define GYRO_CONFIG_VALUE 0x00  // for ±250 degrees per second
#define SAMPLE_RATE_DIV 1       // Sample rate = 1kHz / (1 + 1) = 500Hz

// Other constants
#define PWR_RESET 0x80
#define PWR_WAKE 0x00
#define SENSOR_WAIT 200
#define BAUD_RATE (400 * 1000)
#define SDA_PIN 4
#define SCL_PIN 5
#define SENSOR_DATA_LENGTH 14

template <typename T> struct Vector3D {
public:
  T mX, mY, mZ;

  [[nodiscard]] T magnitude() const {
    return std::sqrt((mX * mX) + (mY * mY) + (mZ * mZ));
  }

  Vector3D<T> operator+(const Vector3D<T> &other) const {
    return {mX + other.mX, mY + other.mY, mZ + other.mZ};
  }

  Vector3D<T> operator-(const Vector3D<T> &other) const {
    return {mX - other.mX, mY - other.mY, mZ - other.mZ};
  }

  Vector3D<T> operator*(T scalar) const {
    return {mX * scalar, mY * scalar, mZ * scalar};
  }

  Vector3D<T> operator/(T scalar) const {
    return {mX / scalar, mY / scalar, mZ / scalar};
  }
};

struct MPU6050Data {
  Vector3D<int16_t> mAccel;
  int16_t mTemp;
  Vector3D<int16_t> mGyro;
};

class MPU6050 {
public:
  MPU6050();

  ~MPU6050() = default;

  // Deleted special member functions to prevent copying
  MPU6050(const MPU6050 &) = delete;
  MPU6050 &operator=(const MPU6050 &) = delete;

  // Move operations to allow transferring ownership
  MPU6050(MPU6050 &&) noexcept = default;
  MPU6050 &operator=(MPU6050 &&) noexcept = default;

  void update();

  [[nodiscard]] Vector3D<int16_t> getAccel() const;

  [[nodiscard]] Vector3D<int16_t> getGyro() const;

  [[nodiscard]] float getTemp() const;

private:
  static void mpu6050Configure();
  static void mpu6050Reset();

  MPU6050Data mSensorData{};

  void mpu6050ReadRaw();
};

#endif // INCLUDE_MPU6050_HPP_
