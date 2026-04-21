cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(HW6 C CXX ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

pico_sdk_init()

add_executable(HW6
    main.c
    usb_descriptors.c
)

target_include_directories(HW6 PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
)

target_link_libraries(HW6
    pico_stdlib
    pico_unique_id
    tinyusb_device
    tinyusb_board
    hardware_i2c
    hardware_gpio
)

pico_add_extra_outputs(HW6)