#include "app/app.hpp"

int main() {
  const auto target = "255.255.255.255";
  const auto port = 3333;

  App app(target, port);

  if (app.initialize()) {
    app.run();
  }

  return 0;
}
