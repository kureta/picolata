#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <lwip/netif.h>
#include <lwip/udp.h>
#include <string>
#include <sys/_intsup.h>

class Network {
public:
  Network() = delete;
  static bool initialize();
  static void deinitialize();
  static bool connectWifi();
  static bool setupUdp();
  static bool send(const char *message, std::string &destAddr,
                   unsigned int port, unsigned int Len);

private:
  static void netifStatusCallback(struct netif *netif);
  static unsigned int Port;
  static ip_addr_t IpAddr;
  static struct udp_pcb *PCB;
};

#endif // NETWORK_HPP
