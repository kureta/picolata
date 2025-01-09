#include "app.hpp"
#include "hardware/adc.h"
#include "lwip/ip_addr.h"
#include "lwip/udp.h"
#include "pico/cyw43_arch.h"
#include "pico/stdlib.h" // IWYU pragma: keep

// TODO: Separate network and OSC logic
App::App(const char *target, const uint16_t port)
    : target(target), port(port) {}

bool App::initialize() {
  stdio_init_all();
  sleep_ms(1000);
  printf("Initializing Wi-Fi...\n");

  adc_init();

  // Make sure GPIO is high-impedance, no pullups etc
  adc_gpio_init(26);
  // Select ADC input 0 (GPIO26)
  adc_select_input(0);

  if (cyw43_arch_init()) {
    printf("Failed to initialise!\n");
    return false;
  }

  cyw43_arch_enable_sta_mode();
  struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];
  netif_set_status_callback(netif, netif_status_callback);

  printf("Connecting to %s...\n", WIFI_SSID);
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
  // tinyOSC instance
  TinyOSC tiny_osc;

  // send the data out of the socket
  // Create a new UDP protocol control block
  struct udp_pcb *pcb = udp_new();
  if (!pcb) {
    printf("Could not create PCB\n");
    return;
  }

  // Prepare the broadcast IP address
  ip_addr_t broadcast_addr;
  IP4_ADDR(&broadcast_addr, 255, 255, 255, 255); // Adjust to your subnet

  while (true) {
    const float conversion_factor = 1.0f / (1 << 12);
    uint16_t result = adc_read();
    printf("Raw value: 0x%03x, voltage: %f V\n", result,
           result * conversion_factor);
    float value = result * conversion_factor;
    // Bind PCB to a port (e.g., 12345)
    if (udp_bind(pcb, IP_ADDR_ANY, 3333) != ERR_OK) {
      printf("Bind failed\n");
      udp_remove(pcb);
      return;
    }

    // write the OSC packet to the buffer
    // returns the number of bytes written to the buffer, negative on error
    // note that tosc_write will clear the entire buffer before writing to it
    const unsigned int len = tiny_osc.writeMessage(
        buffer, sizeof(buffer), "/ping", "fsi", value, "hello", 7);

    struct pbuf *p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);
    if (!p) {
      printf("Failed to allocate buffer\n");
      udp_remove(pcb);
      return;
    }
    memcpy(p->payload, buffer, len);

    udp_sendto(pcb, p, &broadcast_addr, 3333);
    pbuf_free(p);
    sleep_ms(5);
  }
}

App::~App() { cyw43_arch_deinit(); }

void App::netif_status_callback(struct netif *netif) {
  if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
    printf("Network is up\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
  } else {
    printf("Network is down\n");
  }
}
