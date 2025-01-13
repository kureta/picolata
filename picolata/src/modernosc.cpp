#include "modernosc.hpp"
#include "UDPSender.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <lwip/def.h>
#include <lwip/inet.h>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <variant>
#include <vector>

#include <pico/stdlib.h> // IWYU pragma: keep

// Parse an OSC message from a buffer
OscMessage OscParser::parseOscMessage(std::span<const char> data) {
  // Read the OSC Address Pattern
  auto [address, rest] = readPaddedString(data);
  if (address.empty() || address[0] != '/') {
    std::cout << "Invalid OSC address pattern\n";
  }

  // Read the OSC Type Tag String
  auto [typeTags, argsData] = readPaddedString(rest);
  if (typeTags.empty() || typeTags[0] != ',') {
    std::cout << "Invalid OSC type tag string\n";
  }

  // Parse the arguments
  // TODO: Add other types from the specification
  std::vector<OscArgument> Arguments;
  std::string_view Tags = typeTags.substr(1); // Skip the comma
  for (char Tag : Tags) {
    switch (Tag) {
    case 'i': {
      auto [value, nextData] = readInt32(argsData);
      Arguments.emplace_back(value);
      argsData = nextData;
      break;
    }
    case 'f': {
      auto [value, nextData] = readFloat32(argsData);
      Arguments.emplace_back(value);
      argsData = nextData;
      break;
    }
    case 's': {
      auto [strValue, nextData] = readPaddedString(argsData);
      Arguments.emplace_back(std::string{strValue});
      argsData = nextData;
      break;
    }
    default:
      std::cout << "Unsupported OSC type tag\n";
    }
  }

  return OscMessage{.mAddress = std::string{address},
                    .mTypeTags = std::string{typeTags},
                    .mArguments = std::move(Arguments)};
}

// Utility function to read a padded string
std::tuple<std::string_view, std::span<const char>>
OscParser::readPaddedString(std::span<const char> data) {
  auto CharIterator = std::ranges::find(data, '\0');
  if (CharIterator == data.end()) {
    std::cout << "Unterminated string in OSC message\n";
  }
  size_t Len = std::distance(data.begin(), CharIterator) + 1;
  size_t PaddedLen = ((Len + 3) / 4) * 4;
  return {std::string_view{data.data(), Len - 1}, data.subspan(PaddedLen)};
}

// Utility function to read a 32-bit big-endian integer
std::tuple<int32_t, std::span<const char>>
OscParser::readInt32(std::span<const char> data) {
  int32_t Value = 0;
  std::memcpy(&Value, data.data(), sizeof(Value));
  Value = ntohl(Value);
  return {Value, data.subspan(sizeof(int32_t))};
}

// Utility function to read a 32-bit big-endian float
std::tuple<float, std::span<const char>>
OscParser::readFloat32(std::span<const char> data) {
  uint32_t IntBits = 0;
  std::memcpy(&IntBits, data.data(), sizeof(IntBits));
  IntBits = ntohl(IntBits);
  float Value = NAN;
  std::memcpy(&Value, &IntBits, sizeof(Value));
  return {Value, data.subspan(sizeof(float))};
}

OscBuilder::OscBuilder(const std::string &ipAddress, unsigned int port) {
  mSender = std::make_unique<UDPSender>(ipAddress, port);
}

// Function to build an OSC packet from an OscMessage object
std::vector<char> OscBuilder::buildOscMessage(const OscMessage &message) {
  std::vector<char> Buffer;

  // Write the OSC Address Pattern
  writePaddedString(Buffer, message.mAddress);

  // Write the OSC Type Tag String
  writePaddedString(Buffer, message.mTypeTags);

  // Write the arguments
  for (const auto &Arg : message.mArguments) {
    std::visit(
        [&Buffer](const auto &value) {
          using T = std::decay_t<decltype(value)>;
          if constexpr (std::is_same_v<T, int32_t>) {
            writeInt32(Buffer, value);
          } else if constexpr (std::is_same_v<T, float>) {
            writeFloat32(Buffer, value);
          } else if constexpr (std::is_same_v<T, std::string>) {
            writePaddedString(Buffer, value);
          }
        },
        Arg);
  }

  return Buffer;
}

// Helper functions to convert various types to OscArgument
OscArgument OscBuilder::toOscArgument(int value) {
  return OscArgument{static_cast<int32_t>(value)};
}

OscArgument OscBuilder::toOscArgument(float value) {
  return OscArgument{value};
}

OscArgument OscBuilder::toOscArgument(double value) {
  // Convert double to float for OSC float32 type
  return OscArgument{static_cast<float>(value)};
}

OscArgument OscBuilder::toOscArgument(const std::string &value) {
  return OscArgument{value};
}

OscArgument OscBuilder::toOscArgument(const char *value) {
  return OscArgument{std::string{value}};
}

// Utility function to write a padded string
void OscBuilder::writePaddedString(std::vector<char> &buffer,
                                   std::string_view str) {
  size_t Len = str.size() + 1; // Include null terminator
  size_t PaddedLen = ((Len + 3) / 4) * 4;

  buffer.insert(buffer.end(), str.begin(), str.end());
  buffer.push_back('\0'); // Null terminator

  // Pad with null bytes to align to 4 bytes
  buffer.insert(buffer.end(), PaddedLen - Len, '\0');
}

// Utility function to write a 32-bit big-endian integer
void OscBuilder::writeInt32(std::vector<char> &buffer, int32_t value) {
  value = htonl(value);
  std::array<char, sizeof(int32_t)> Bytes{};
  std::memcpy(Bytes.data(), &value, sizeof(value));
  buffer.insert(buffer.end(), std::begin(Bytes), std::end(Bytes));
}

// Utility function to write a 32-bit big-endian float
void OscBuilder::writeFloat32(std::vector<char> &buffer, float value) {
  uint32_t IntBits = 0;
  std::memcpy(&IntBits, &value, sizeof(value));
  IntBits = htonl(IntBits);
  std::array<char, sizeof(uint32_t)> Bytes{};
  std::memcpy(Bytes.data(), &IntBits, sizeof(IntBits));
  buffer.insert(buffer.end(), std::begin(Bytes), std::end(Bytes));
}
