cmake_minimum_required(VERSION 3.17)

# ====== set board and import pico-sdk ======
set(PICO_BOARD pico_w)

# initialize pico-sdk from GIT
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_FETCH_FROM_GIT on)
set(PICO_SDK_FETCH_FROM_GIT_TAG 2.1.0)

# pico_sdk_import.cmake is a single file copied from this SDK
# note: this must happen before project()
include(cmake/pico_sdk_import.cmake)

# ======= define project ======
project(picolata C CXX)

# Temporary hack for clangd
add_compile_definitions(_LIBCPP_HAS_THREAD_API_PTHREAD=1)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_SYSROOT /usr/arm-none-eabi)
set(CMAKE_C_COMPILER /usr/bin/arm-none-eabi-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-none-eabi-g++)

# initialize the Raspberry Pi Pico SDK
pico_sdk_init()

# ====== Inject secrets ======
add_compile_definitions(WIFI_SSID="$ENV{WIFI_SSID}")
add_compile_definitions(WIFI_PASS="$ENV{WIFI_PASS}")

# ======= define targets =======
set(SOURCE_DIR "${CMAKE_SOURCE_DIR}/picolata")
file(GLOB SOURCE_FILES "${SOURCE_DIR}/src/*.cpp" "${SOURCE_DIR}/src/*.c")

add_executable(${CMAKE_PROJECT_NAME} ${SOURCE_DIR}/main.cpp ${SOURCE_FILES})

target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE ${SOURCE_DIR}/include)

target_link_libraries(
    ${CMAKE_PROJECT_NAME}
    pico_stdlib
    pico_cyw43_arch_lwip_threadsafe_background
    hardware_adc
    hardware_i2c
    tinyusb_device
    tinyusb_board
)

pico_enable_stdio_usb(${CMAKE_PROJECT_NAME} 0)
pico_enable_stdio_uart(${CMAKE_PROJECT_NAME} 1)

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
