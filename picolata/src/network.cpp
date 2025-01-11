#include "network.hpp"
#include <iostream>
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h> // IWYU pragma: keep
#include <sys/_types.h>

#define CONNECTION_TIMEOUT 10000
#define BIND_PORT 3333

struct udp_pcb *Network::PCB = nullptr;

// TODO: move send logic into a different class (maybe Socket)
bool Network::connectWifi() {
  std::cout << "Initializing Wi-Fi...\n";

  if (cyw43_arch_init() != 0) {
    std::cout << "Failed to initialize!\n";
    return false;
  }
  // Enable Wi-Fi Station mode
  cyw43_arch_enable_sta_mode();
  // Set up a network status callback
  struct netif *Netif = &cyw43_state.netif[CYW43_ITF_STA];
  netif_set_status_callback(Netif, netifStatusCallback);

  std::cout << "Connecting to " << WIFI_SSID << "...\n";
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                         CYW43_AUTH_WPA2_AES_PSK,
                                         CONNECTION_TIMEOUT) != 0) {
    std::cout << "Failed to connect!\n";
    return false;
  }
  std::cout << "Connected.\n";
  return true;
}

bool Network::setupUdp() {
  // Create a new UDP protocol control block
  PCB = udp_new();
  if (PCB == nullptr) {
    std::cout << "Could not create PCB\n";
    return false;
  }

  // Bind PCB to a port (e.g., 12345)
  if (udp_bind(PCB, IP_ADDR_ANY, BIND_PORT) != ERR_OK) {
    std::cout << "Bind failed\n";
    udp_remove(PCB);
    return false;
  }

  return true;
}

bool Network::initialize() {
  if (!Network::connectWifi()) {
    return false;
  }

  if (!Network::setupUdp()) {
    return false;
  };

  return true;
}

void Network::deinitialize() {
  udp_remove(PCB);
  cyw43_arch_deinit();
}

bool Network::send(const char *message, std::string &destAddr,
                   unsigned int port, const unsigned int Len) {
  // Parse address
  ip_addr_t BroadcastAddr;
  ipaddr_aton(destAddr.c_str(), &BroadcastAddr);

  // Setup Protocol Buffer
  struct pbuf *PBuf = pbuf_alloc(PBUF_TRANSPORT, Len, PBUF_RAM);
  if (PBuf == nullptr) {
    std::cout << "Failed to allocate buffer\n";
    return false;
  }

  // Copy message into pbuff
  memcpy(PBuf->payload, message, Len);

  // Send pbuff via UDP
  udp_sendto(PCB, PBuf, &BroadcastAddr, port);

  // Free pbuf
  pbuf_free(PBuf);

  return true;
}

void Network::netifStatusCallback(struct netif *netif) {
  if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
    std::cout << "Network is up\n";
    std::cout << "IP Address: " << ip4addr_ntoa(netif_ip4_addr(netif)) << "\n";
  } else {
    std::cout << "Network is down\n";
    std::cout << "Trying to reconnect in 3 seconds...\n";
    Network::initialize();
  }
}
