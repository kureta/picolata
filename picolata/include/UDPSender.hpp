#ifndef INCLUDE_UDPSENDER_HPP_
#define INCLUDE_UDPSENDER_HPP_

#include "SenderInterface.hpp"
#include <lwip/udp.h>
#include <string>
#include <vector>

class UDPSender : public SenderInterface {
public:
  UDPSender(const std::string &ipAddress, unsigned int port);
  bool send(std::vector<char> &message) override;

private:
  unsigned int mPort;
  ip_addr_t mIpAddr;
  struct udp_pcb *mPcb;
};

#endif // INCLUDE_UDPSENDER_HPP_
