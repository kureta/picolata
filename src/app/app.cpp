#include "app.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h" // IWYU pragma: keep
#include "lwip/udp.h"
#include "lwip/ip_addr.h"

// TODO: Separate network and OSC logic
App::App(const char *target, const uint16_t port)
    : target(target), port(port) {}

bool App::initialize() {
  stdio_init_all();
  sleep_ms(1000);
  printf("Initializing Wi-Fi...\n");

  if (cyw43_arch_init()) {
    printf("Failed to initialise!\n");
    return false;
  }

  cyw43_arch_enable_sta_mode();
  struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];
  netif_set_status_callback(netif, netif_status_callback);

  printf("Connecting to Wi-Fi...\n");
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("Failed to connect!\n");
    return false;
  } else {
    printf("Connected.\n");
    run();
    return true;
  }
}

void App::run() {

  // declare a buffer for writing the OSC packet into
  char buffer[1024];

  // write the OSC packet to the buffer
  // returns the number of bytes written to the buffer, negative on error
  // note that tosc_write will clear the entire buffer before writing to it
  int len = tosc_writeMessage(
      buffer, sizeof(buffer),
      "/ping", // the address
      "fsi",   // the format; 'f':32-bit float, 's':ascii string, 'i':32-bit integer
      1.0f, "hello", 2);

  // send the data out of the socket
  // Create a new UDP protocol control block
  struct udp_pcb *pcb = udp_new();
  if (!pcb) {
    printf("Could not create PCB\n");
    return;
  }

  // Prepare the broadcast IP address
  ip_addr_t broadcast_addr;
  IP4_ADDR(&broadcast_addr, 255, 255, 255, 255);  // Adjust to your subnet

  while (true) {
    // Bind PCB to a port (e.g., 12345)
    if (udp_bind(pcb, IP_ADDR_ANY, 3333) != ERR_OK) {
      printf("Bind failed\n");
      udp_remove(pcb);
      return;
    }

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (!p) {
      printf("Failed to allocate buffer\n");
      udp_remove(pcb);
      return;
    }
    memcpy(p->payload, buffer, len);

    udp_sendto(pcb, p, &broadcast_addr, 3333);
    pbuf_free(p);
    sleep_ms(500);
  }
}

App::~App() {
  cyw43_arch_deinit();
}

void App::netif_status_callback(struct netif *netif) {
  if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
    printf("Network is up\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
  } else {
    printf("Network is down\n");
  }
}
