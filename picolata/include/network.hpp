#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <lwip/netif.h>

class Network {
public:
  Network() = delete;
  static bool initialize();
  static void deinitialize();
  static bool connectWifi();

private:
  static void netifStatusCallback(struct netif *netif);
};

#endif // NETWORK_HPP
