/**
 * Copyright (c) 2022 Andrew McDonnell
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

// TODO: Use tinyosc. It does not have any networking code, which is good for
// us.
//       Find out relevant "byte order convertion functions, such as htonl,
//       ntohl, htons, and ntohs, which are used in the code to ensure data is
//       correctly formatted for network transmission. "
//       Look at the netwworking code of PicoOSC

#include "pico/cyw43_arch.h"
#include "pico/stdlib.h" // IWYU pragma: keep

#include "lwip/pbuf.h"
#include "lwip/udp.h"

#include "tinyosc.h"

#define UDP_PORT 4444
#define BEACON_MSG_LEN_MAX 127
#define BEACON_TARGET "255.255.255.255"
#define BEACON_INTERVAL_MS 1000

void run_udp_beacon() {
  struct udp_pcb *pcb = udp_new();

  ip_addr_t addr;
  ipaddr_aton(BEACON_TARGET, &addr);

  int counter = 0;
  while (true) {
    struct pbuf *p =
        pbuf_alloc(PBUF_TRANSPORT, BEACON_MSG_LEN_MAX + 1, PBUF_RAM);

    char buffer[2048]; // declare a 2Kb buffer to read packet data into

    printf("Starting write tests:\n");
    int len = 0;
    char blob[8] = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF};
    len = tosc_writeMessage(buffer, sizeof(buffer), "/address", "fsibTFNI",
                            1.0f, "hello world", -1, sizeof(blob), blob);
    tosc_printOscBuffer(buffer, len);
    p->payload = buffer;
    printf("done.\n");

    char *req = (char *)p->payload;
    memset(req, 0, BEACON_MSG_LEN_MAX + 1);
    snprintf(req, BEACON_MSG_LEN_MAX, "%d\n", counter);
    err_t er = udp_sendto(pcb, p, &addr, UDP_PORT);
    pbuf_free(p);
    if (er != ERR_OK) {
      printf("Failed to send UDP packet! error=%d", er);
    } else {
      printf("Sent packet %d\n", counter);
      counter++;
    }

    // Note in practice for this simple UDP transmitter,
    // the end result for both background and poll is the same

#if PICO_CYW43_ARCH_POLL
    // if you are using pico_cyw43_arch_poll, then you must poll periodically
    // from your main loop (not from a timer) to check for Wi-Fi driver or lwIP
    // work that needs to be done.
    cyw43_arch_poll();
    sleep_ms(BEACON_INTERVAL_MS);
#else
    // if you are not using pico_cyw43_arch_poll, then WiFI driver and lwIP work
    // is done via interrupt in the background. This sleep is just an example of
    // some (blocking) work you might be doing.
    sleep_ms(BEACON_INTERVAL_MS);
#endif
  }
}

static void netif_status_callback(struct netif *netif) {
  if (netif_is_up(netif) && !ip4_addr_isany_val(*netif_ip4_addr(netif))) {
    printf("Network is up\n");
    printf("IP Address: %s\n", ip4addr_ntoa(netif_ip4_addr(netif)));
  } else {
    printf("Network is down\n");
  }
}

int main() {
  stdio_init_all();

  if (cyw43_arch_init()) {
    printf("failed to initialise\n");
    return 1;
  }

  cyw43_arch_enable_sta_mode();

  struct netif *netif = &cyw43_state.netif[CYW43_ITF_STA];
  netif_set_status_callback(netif, netif_status_callback);

  printf("Connecting to Wi-Fi...\n");
  if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASS,
                                         CYW43_AUTH_WPA2_AES_PSK, 30000)) {
    printf("failed to connect.\n");
    return 1;
  } else {
    printf("Connected.\n");
  }
  run_udp_beacon();
  cyw43_arch_deinit();
  return 0;
}
