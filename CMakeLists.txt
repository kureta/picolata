cmake_minimum_required(VERSION 3.17)

# ====== set board and import pico-sdk ======
set(PICO_BOARD pico_w)
include(cmake/pico_sdk_import.cmake)

# ======= define project ======
project(picolata C CXX)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# ======= define targets =======
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

# ====== installation command ======
# Define the path to the output UF2 file
set(OUTPUT_UF2 "${CMAKE_BINARY_DIR}/blink_simple.uf2")

install(
    CODE
        "
execute_process(
COMMAND picotool load -v -x ${OUTPUT_UF2} -f
WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
RESULT_VARIABLE result
)
if(NOT result EQUAL 0)
message(FATAL_ERROR \"Picotool command failed\")
endif()
"
)
