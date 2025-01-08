#ifndef APP_HPP
#define APP_HPP

#include "PicoOSC.hpp"

class App {
public:
  App(const char *target, const uint16_t port);
  bool initialize();
  void run();
  ~App();

private:
  const char *target;
  uint16_t port;
  picoosc::OSCClient *client;
  static void netif_status_callback(struct netif *netif);
};

#endif // APP_HPP
