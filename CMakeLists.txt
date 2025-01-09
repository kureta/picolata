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
add_compile_definitions(WIFI_SSID="$ENV{WIFI_SSID}")
add_compile_definitions(WIFI_PASS="$ENV{WIFI_PASS}")

add_executable(
    ${CMAKE_PROJECT_NAME}
    src/main.cpp
    src/app/app.cpp
    src/tinyosc/tinyosc.cpp
)

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE src/include src)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
)

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 1)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 0)

# create map/bin/hex/uf2 file etc.
pico_add_extra_outputs(${CMAKE_PROJECT_NAME})

# ====== installation command ======
# Define the path to the output UF2 file
set(OUTPUT_UF2 "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.uf2")

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
