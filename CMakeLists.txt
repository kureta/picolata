cmake_minimum_required(VERSION 3.17)

# ====== Fetch and set up dependencies ======
include(cmake/pico_sdk_import.cmake)

# =======
project(picolata C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# ======= Our Code =======
set(PICO_BOARD pico_w)

add_executable(blink_simple src/main.c)

target_include_directories(blink_simple PRIVATE src/include)

target_link_libraries(
    blink_simple
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
)

pico_enable_stdio_usb(blink_simple 1)
pico_enable_stdio_uart(blink_simple 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(blink_simple)
