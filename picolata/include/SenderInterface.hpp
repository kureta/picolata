#ifndef SENDERINTERFACE_HPP
#define SENDERINTERFACE_HPP

#include <vector>

class SenderInterface {
public:
  // Use defult constructor and destructor
  SenderInterface() = default;
  virtual ~SenderInterface() = default;

  // Deleted special member functions to prevent copying
  SenderInterface(const SenderInterface &) = delete;
  SenderInterface &operator=(const SenderInterface &) = delete;

  // Use default move operations to allow transferring ownership
  SenderInterface(SenderInterface &&) noexcept = delete;
  SenderInterface &operator=(SenderInterface &&) noexcept = delete;

  // This is the only thing we need the interface to have.
  virtual bool send(std::vector<char> &data) = 0;
};

#endif // SENDERINTERFACE_HPP
