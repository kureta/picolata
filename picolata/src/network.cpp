#include "network.hpp"
#include <lwip/ip_addr.h>
#include <lwip/pbuf.h>
#include <pico/cyw43_arch.h>
#include <pico/stdlib.h> // IWYU pragma: keep

#define BUFFER_LENGTH 1024

struct udp_pcb *Network::pcb = nullptr;

bool Network::connect_wifi() {
  printf("Initializing Wi-Fi...\n");

  if (cyw43_arch_init()) {
    printf("Failed to initialize!\n");
    return false;
  }
  // Enable Wi-Fi Station mode
  cyw43_arch_enable_sta_mode();
  // Set up a network status callback
  struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];
  netif_set_status_callback(netif, netif_status_callback);

  printf("Connecting to %s...\n", WIFI_SSID);
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("Failed to connect!\n");
    return false;
  } else {
    printf("Connected.\n");
    return true;
  }
}

bool Network::setup_udp() {
  // Create a new UDP protocol control block
  pcb = udp_new();
  if (!pcb) {
    printf("Could not create PCB\n");
    return false;
  }

  // Bind PCB to a port (e.g., 12345)
  if (udp_bind(pcb, IP_ADDR_ANY, 3333) != ERR_OK) {
    printf("Bind failed\n");
    udp_remove(pcb);
    return false;
  }

  return true;
}

bool Network::initialize() {
  if (!Network::connect_wifi()) {
    return false;
  }

  if (!Network::setup_udp()) {
    return false;
  };

  return true;
}

void Network::deinitialize() {
  udp_remove(pcb);
  cyw43_arch_deinit();
}

bool Network::send(const char *message, const char *dest_addr,
                   const unsigned int port) {
  // Start
  const unsigned int len = strlen(message) * sizeof(message);

  // Parse address
  ip_addr_t broadcast_addr;
  ipaddr_aton(dest_addr, &broadcast_addr);

  // Setup Protocol Buffer
  struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
  if (!p) {
    printf("Failed to allocate buffer\n");
    return false;
  }

  // Copy message into pbuff
  memcpy(p->payload, message, len);

  // Send pbuff via UDP
  udp_sendto(pcb, p, &broadcast_addr, port);

  // Free pbuf
  pbuf_free(p);

  return true;
}

void Network::netif_status_callback(struct netif *netif) {
  if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
    printf("Network is up\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
  } else {
    printf("Network is down\n");
    printf("Trying to reconnect in 3 seconds...");
    Network::initialize();
  }
}
