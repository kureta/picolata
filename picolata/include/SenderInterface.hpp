#ifndef SENDERINTERFACE_HPP
#define SENDERINTERFACE_HPP

#include <vector>

class SenderInterface {
public:
  // Don't *totally* know why these are necessary
  SenderInterface() = default;
  virtual ~SenderInterface() = default;

  // Don't know why these are necessary. Shut up `clang-tidy`
  SenderInterface(const SenderInterface &) = delete;
  SenderInterface(SenderInterface &&) = delete;
  SenderInterface &operator=(const SenderInterface &) = delete;
  SenderInterface &operator=(SenderInterface &&) = delete;

  // This is the only thing we need the interface to have.
  virtual bool send(std::vector<char> &data) = 0;
};

#endif // SENDERINTERFACE_HPP
