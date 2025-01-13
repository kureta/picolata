#include "UDPSender.hpp"
#include <cstring>
#include <iostream>
#include <lwip/udp.h>
#include <vector>

#define BIND_PORT 3333

UDPSender::UDPSender(const std::string &ipAddress, unsigned int port)
    : mIpAddr(0), mPort(port), mPcb(udp_new()) {

  // Parse the destination IP address
  ipaddr_aton(ipAddress.c_str(), &mIpAddr);

  // Check if UDP Protocol Control Block can be created
  if (mPcb == nullptr) {
    std::cout << "Could not create UDP Protocol Control Block\n";
  }

  // Bind PCB to a port
  if (udp_bind(mPcb, IP_ADDR_ANY, BIND_PORT) != ERR_OK) {
    std::cout << "Bind failed\n";
    udp_remove(mPcb);
  }
}
bool UDPSender::send(std::vector<char> &message) {
  // Setup Protocol Buffer
  struct pbuf *PBuf = pbuf_alloc(PBUF_TRANSPORT, message.size(), PBUF_RAM);
  if (PBuf == nullptr) {
    std::cout << "Failed to allocate buffer\n";
    return false;
  }

  // Copy message into pbuff
  memcpy(PBuf->payload, message.data(), message.size());

  // Send pbuff via UDP
  udp_sendto(mPcb, PBuf, &mIpAddr, mPort);

  // Free pbuf
  pbuf_free(PBuf);

  return true;
}
