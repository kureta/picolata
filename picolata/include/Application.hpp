#ifndef INCLUDE_APPLICATION_HPP_
#define INCLUDE_APPLICATION_HPP_

#include "MPU6050_6Axis_MotionApps_V6_12.h" // IWYU pragma: keep
#include "adc.hpp"
#include "modernosc.hpp"
#include <cstdint>
#include <memory>

class Application {
public:
  Application();
  void update(uint32_t delta);
  [[nodiscard]] unsigned int getUpdateRate() const { return mUpdateRate; }

private:
  unsigned int mUpdateRate;
  std::unique_ptr<OscBuilder> mOSC;
  ADC mAdc;
  MPU6050 mMpu6050;
};

#endif // INCLUDE_APPLICATION_HPP_
