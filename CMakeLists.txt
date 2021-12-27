cmake_minimum_required(VERSION 3.12)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(picojoy C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(picojoy
    src/main.c
    src/usb_descriptors.c)

target_include_directories(picojoy PRIVATE include/)

target_link_libraries(picojoy PRIVATE pico_stdlib tinyusb_device tinyusb_board pico_unique_id)
pico_add_extra_outputs(picojoy)
