#ifndef POT_HPP
#define POT_HPP

#define ADC_RESOLUTION 4096

class ADC {
public:
  ADC() = delete;
  static void initialize();
  static float getValue();
};

#endif // POT_HPP
