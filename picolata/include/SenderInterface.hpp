#ifndef SENDERINTERFACE_HPP
#define SENDERINTERFACE_HPP

#include <vector>

class SenderInterface {
public:
  SenderInterface() = default;
  SenderInterface(const SenderInterface &) = delete;
  SenderInterface(SenderInterface &&) = delete;
  SenderInterface &operator=(const SenderInterface &) = default;
  SenderInterface &operator=(SenderInterface &&) = delete;
  virtual ~SenderInterface() = default;
  virtual bool send(std::vector<char> &data) = 0;
};

#endif // SENDERINTERFACE_HPP
