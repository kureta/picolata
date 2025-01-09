#ifndef POT_HPP
#define POT_HPP

class ADC {
public:
  ADC();
  ~ADC() = default;
  float get_value();

private:
  const float conversion_factor = 1.0f / (1 << 12);
};

#endif // POT_HPP
