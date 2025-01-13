#ifndef POT_HPP
#define POT_HPP

#include <array>
#define ADC_RESOLUTION 4096

class ADC {
public:
  explicit ADC(int adcIndex);
  static float getValue();

private:
  int mAdcIndex;
  // Define a static constant map from int to int
  static const std::array<int, 3> kAdcToGpio;
};

#endif // POT_HPP
