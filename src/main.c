#include "pico/cyw43_arch.h"
#include "pico/stdlib.h" // IWYU pragma: keep

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#define PORT 8080

static err_t recv_callback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p,
                           err_t err) {
  if (p == NULL) {
    tcp_close(tpcb);
    return ERR_OK;
  }

  tcp_recved(tpcb, p->tot_len);
  tcp_write(tpcb, p->payload, p->len, TCP_WRITE_FLAG_COPY);
  printf("Received: %.*s\n", p->len, (char *)p->payload);
  pbuf_free(p);

  return ERR_OK;
}

static err_t accept_callback(void *arg, struct tcp_pcb *newpcb, err_t err) {
  tcp_recv(newpcb, recv_callback);
  return ERR_OK;
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
  sleep_ms(1000);
  printf("Woke!\n");

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

  struct tcp_pcb *pcb = tcp_new();
  tcp_bind(pcb, IP_ADDR_ANY, PORT);
  pcb = tcp_listen(pcb);
  tcp_accept(pcb, accept_callback);

  printf("TCP echo server started on port %d\n", PORT);

  while (true) {
    cyw43_arch_poll();
  }

  return 0;
}
