#ifndef MODERN_OSC_HPP
#define MODERN_OSC_HPP

#include "SenderInterface.hpp"
#include <cstdint>
#include <memory>
#include <span>
#include <string>
#include <variant>
#include <vector>

// Define an alias for the OSC argument types
using OscArgument = std::variant<int32_t, float, std::string>;

// OSC Message structure
struct OscMessage {
  std::string mAddress;
  std::string mTypeTags;
  std::vector<OscArgument> mArguments;
};

class OscParser {
public:
  OscParser() = delete;
  // Parse an OSC message from a buffer
  static OscMessage parseOscMessage(std::span<const char> data);

private:
  // Utility function to read a padded string
  static std::tuple<std::string_view, std::span<const char>>
  readPaddedString(std::span<const char> data);

  // Utility function to read a 32-bit big-endian integer
  static std::tuple<int32_t, std::span<const char>>
  readInt32(std::span<const char> data);

  // Utility function to read a 32-bit big-endian float
  static std::tuple<float, std::span<const char>>
  readFloat32(std::span<const char> data);
};

class OscBuilder {
public:
  explicit OscBuilder(const std::string &ipAddress, unsigned int port);

  // Function for directly building an OSC packet from address and arguments
  template <typename... Args>
  void sendOSCMessage(const std::string &address, Args &&...args) {
    std::vector<char> Msg =
        buildOscMessage(createOscMessage(address, std::forward<Args>(args)...));
    mSender->send(Msg);
  }

private:
  // Function to create an OscMessage with any number of arguments
  template <typename... Args>
  static OscMessage createOscMessage(const std::string &address,
                                     Args &&...args) {
    OscMessage Message;
    Message.mAddress = address;
    Message.mArguments.reserve(sizeof...(Args));

    // Convert and store arguments
    (Message.mArguments.emplace_back(toOscArgument(std::forward<Args>(args))),
     ...);

    // Build the type tag string
    Message.mTypeTags = ",";
    for (const auto &Arg : Message.mArguments) {
      std::visit(
          [&Message](const auto &value) {
            using T = std::decay_t<decltype(value)>;
            if constexpr (std::is_same_v<T, int32_t>) {
              Message.mTypeTags += 'i';
            } else if constexpr (std::is_same_v<T, float>) {
              Message.mTypeTags += 'f';
            } else if constexpr (std::is_same_v<T, std::string>) {
              Message.mTypeTags += 's';
            }
          },
          Arg);
    }

    return Message;
  }

  // Function to build an OSC packet from an OscMessage object
  static std::vector<char> buildOscMessage(const OscMessage &message);
  // Sender must have a send method that accepts a buffer
  std::unique_ptr<SenderInterface> mSender;

  // Helper functions to convert various types to OscArgument
  static OscArgument toOscArgument(int value);
  static OscArgument toOscArgument(float value);
  static OscArgument toOscArgument(double value);
  static OscArgument toOscArgument(const std::string &value);
  static OscArgument toOscArgument(const char *value);

  // Utility function to write a padded string
  static void writePaddedString(std::vector<char> &buffer,
                                std::string_view str);

  // Utility function to write a 32-bit big-endian integer
  static void writeInt32(std::vector<char> &buffer, int32_t value);

  // Utility function to write a 32-bit big-endian float
  static void writeFloat32(std::vector<char> &buffer, float value);
};

#endif // MODERN_OSC_HPP
