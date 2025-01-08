include(FetchContent)
FetchContent_Declare(
    pico_sdk
    GIT_REPOSITORY https://github.com/raspberrypi/pico-sdk
    GIT_TAG master
    GIT_SUBMODULES_RECURSE FALSE
)

message("Downloading Raspberry Pi Pico SDK")
FetchContent_Populate(pico_sdk)

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
if(NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(
        FATAL_ERROR
        "Directory '${PICO_SDK_PATH}' does not appear to contain the Raspberry Pi Pico SDK"
    )
endif()

include(${PICO_SDK_INIT_CMAKE_FILE})
