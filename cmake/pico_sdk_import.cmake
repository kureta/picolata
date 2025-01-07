include(FetchContent)
set(FETCHCONTENT_BASE_DIR_SAVE ${FETCHCONTENT_BASE_DIR})
FetchContent_Declare(
    pico_sdk
    GIT_REPOSITORY https://github.com/raspberrypi/pico-sdk
    GIT_TAG ${PICO_SDK_FETCH_FROM_GIT_TAG}
    GIT_SUBMODULES_RECURSE FALSE
)

if(NOT pico_sdk)
    message("Downloading Raspberry Pi Pico SDK")
    FetchContent_Populate(pico_sdk)
    set(PICO_SDK_PATH ${pico_sdk_SOURCE_DIR})
endif()
set(FETCHCONTENT_BASE_DIR ${FETCHCONTENT_BASE_DIR_SAVE})

get_filename_component(
    PICO_SDK_PATH
    "${PICO_SDK_PATH}"
    REALPATH
    BASE_DIR "${CMAKE_BINARY_DIR}"
)
if(NOT EXISTS ${PICO_SDK_PATH})
    message(FATAL_ERROR "Directory '${PICO_SDK_PATH}' not found")
endif()

set(PICO_SDK_INIT_CMAKE_FILE ${PICO_SDK_PATH}/pico_sdk_init.cmake)
if(NOT EXISTS ${PICO_SDK_INIT_CMAKE_FILE})
    message(
        FATAL_ERROR
        "Directory '${PICO_SDK_PATH}' does not appear to contain the Raspberry Pi Pico SDK"
    )
endif()

set(PICO_SDK_PATH
    ${PICO_SDK_PATH}
    CACHE PATH
    "Path to the Raspberry Pi Pico SDK"
    FORCE
)

include(${PICO_SDK_INIT_CMAKE_FILE})
