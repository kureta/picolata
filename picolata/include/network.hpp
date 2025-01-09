#ifndef NETWORK_HPP
#define NETWORK_HPP

#include <lwip/netif.h>
#include <lwip/udp.h>

class Network {
public:
  Network() = delete;
  static bool initialize();
  static void deinitialize();
  static bool connect_wifi();
  static bool setup_udp();
  static bool send(const char *message, const char *dest_addr,
                   const unsigned int port);

private:
  static void netif_status_callback(struct netif *netif);
  static unsigned int port;
  static ip_addr_t ip_addr;
  static struct udp_pcb *pcb;
};

#endif // NETWORK_HPP
