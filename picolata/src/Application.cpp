#include "Application.hpp"
#include "network.hpp"
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <memory>

#define BROADCAST_IP "255.255.255.255"
#define OSC_PORT 3333
#define ONE_SECOND 1000
#define UPDATE_RATE 120
#define ADC_PIN 0

bool dmpReady = false; // set true if DMP init was successful
uint8_t devStatus; // return status after each device operation (0 = success, !0
Quaternion q;      // [w, x, y, z]         quaternion container
VectorInt16 aa;    // [x, y, z]            accel sensor measurements
VectorInt16 gy;    // [x, y, z]            gyro sensor measurements
VectorInt16
    aaReal; // [x, y, z]            gravity-free accel sensor measurements
VectorInt16
    aaWorld; // [x, y, z]            world-frame accel sensor measurements
VectorFloat gravity; // [x, y, z]            gravity vector
float euler[3];      // [psi, theta, phi]    Euler angle container
float
    ypr[3]; // [yaw, pitch, roll]   yaw/pitch/roll container and gravity vector
float yaw, pitch, roll;
uint8_t mpuIntStatus;   // holds actual interrupt status byte from MPU
uint16_t packetSize;    // expected DMP packet size (default is 42 bytes)
uint16_t fifoCount;     // count of all bytes currently in FIFO
uint8_t fifoBuffer[64]; // FIFO storage buffer
volatile bool mpuInterrupt =
    false; // indicates whether MPU interrupt pin has gone high
void dmpDataReady() { mpuInterrupt = true; }

Application::Application() : mUpdateRate(UPDATE_RATE), mAdc(ADC_PIN) {
  // Initialize network
  if (!Network::initialize()) {
    std::cout << "Failed to initialize!\n";
  }

  i2c_init(i2c_default, 400 * 1000);
  gpio_set_function(PICO_DEFAULT_I2C_SDA_PIN, GPIO_FUNC_I2C);
  gpio_set_function(PICO_DEFAULT_I2C_SCL_PIN, GPIO_FUNC_I2C);
  gpio_pull_up(PICO_DEFAULT_I2C_SDA_PIN);
  gpio_pull_up(PICO_DEFAULT_I2C_SCL_PIN);

  // Create OSC sender
  mOSC = std::make_unique<OscBuilder>(BROADCAST_IP, OSC_PORT);
  mMpu6050.initialize();
  devStatus = mMpu6050.dmpInitialize();

  /* --- alternatively you can try this (6 loops should be enough) --- */
  mMpu6050.CalibrateAccel(12);
  mMpu6050.CalibrateGyro(12);

  if (devStatus == 0) {
    mMpu6050.setDMPEnabled(true); // turn on the DMP, now that it's ready
    mpuIntStatus = mMpu6050.getIntStatus();
    dmpReady = true; // set our DMP Ready flag so the main loop() function knows
                     // it's okay to use it
    packetSize = mMpu6050.dmpGetFIFOPacketSize(); // get expected DMP packet
                                                  // size for later comparison
  } else { // ERROR!        1 = initial memory load failed         2 = DMP
           // configuration updates failed        (if it's going to break,
           // usually the code will be 1)
    printf("DMP Initialization failed (code %d)", devStatus);
    sleep_ms(2000);
  }
  yaw = 0.0;
  pitch = 0.0;
  roll = 0.0;
}

// Main loop
void Application::update(uint32_t delta) {
  // Send time passed between loops
  mOSC->sendOSCMessage("/delta", static_cast<float>(delta) / ONE_SECOND);
  // Send pot value
  mOSC->sendOSCMessage("/pot", ADC::getValue());

  mpuInterrupt = true;
  fifoCount = mMpu6050.getFIFOCount(); // get current FIFO count
  if ((mpuIntStatus & 0x10) ||
      fifoCount == 1024) // check for overflow (this should never happen
                         // unless our code is too inefficient)
  {
    mMpu6050.resetFIFO(); // reset so we can continue cleanly
    printf("FIFO overflow!");
  } else if (mpuIntStatus & 0x01) // otherwise, check for DMP data ready
                                  // interrupt (this should happen frequently)
  {
    while (fifoCount < packetSize)
      fifoCount =
          mMpu6050.getFIFOCount(); // wait for correct available data
                                   // length, should be a VERY short wait
    mMpu6050.getFIFOBytes(fifoBuffer, packetSize); // read a packet from FIFO
    fifoCount -= packetSize; // track FIFO count here in case there is > 1
                             // packet available

    // Get data
    mMpu6050.dmpGetQuaternion(&q, fifoBuffer);
    mMpu6050.dmpGetAccel(&aa, fifoBuffer);
    mMpu6050.dmpGetGravity(&gravity, &q);
    mMpu6050.dmpGetYawPitchRoll(ypr, &q, &gravity);
    mMpu6050.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    mMpu6050.dmpGetLinearAccelInWorld(&aaWorld, &aaReal, &q);
    yaw = ypr[0] * 180 / PI;
    pitch = ypr[1] * 180 / PI;
    roll = ypr[2] * 180 / PI;

    // Send data
    mOSC->sendOSCMessage("/quat", q.w, q.x, q.y, q.z);
    mOSC->sendOSCMessage("/accel", aa.x, aa.y, aa.z);
    mOSC->sendOSCMessage("/gravity", aa.x, aa.y, aa.z);
    mOSC->sendOSCMessage("/ypr", aa.x, aa.y, aa.z);
    mOSC->sendOSCMessage("/real_accel", aa.x, aa.y, aa.z);
    mOSC->sendOSCMessage("/world_accel", aa.x, aa.y, aa.z);
  }
}
