#include "app.hpp"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h" // IWYU pragma: keep
#include <stdio.h>

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
    client = new picoosc::OSCClient(target, port);
    return true;
  }
}

void App::run() {
  if (!client) {
    printf("Client not initialized.\n");
    return;
  }

  picoosc::OSCMessage msg;
  while (true) {
    msg.addAddress("/cont_1");
    msg.add<float>(0.01);
    msg.send(*client);
    msg.clear();

    msg.addAddress("/cont_2");
    msg.add<float>(0.7);
    msg.send(*client);
    msg.clear();

    sleep_ms(500);
  }
}

App::~App() {
  if (client) {
    delete client;
  }
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
