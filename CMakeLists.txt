cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(picogbox)
pico_sdk_init()
add_definitions(-DPICO)
set(DBG_PIN_COUNT=4)

add_executable(picogbox
        src/log.c
        src/pico_led.c
        src/pico_serialid.c
        src/tusb_descriptors.c
        src/bin_cdc_microshell.c
        src/bin_cdc_bridge_uart.c
        src/bin_cdc_sump.c
        src/bin_cdc_sigrock.c
        src/bin_vendor_swd.c
        src/picogbox.c
)

if (DEFINED ENV{TURBO_200MHZ})
        message("TURBO_200MHZ is defined as " $ENV{TURBO_200MHZ})
        target_compile_definitions(picogbox PRIVATE TURBO_200MHZ=$ENV{TURBO_200MHZ})
endif()

include(src/microshell/CMakeLists.txt)
build_ush(ush)

target_include_directories(picogbox PRIVATE 
                        src
                        )

target_link_libraries(picogbox PRIVATE 
                        pico_stdlib 
                        pico_unique_id
                        tinyusb_device 
                        tinyusb_board 
                        hardware_pio 
                        hardware_dma 
                        hardware_pwm
                        ush
                        )

pico_generate_pio_header(picogbox ${CMAKE_CURRENT_LIST_DIR}/src/bin_vendor_swd.pio)
pico_add_extra_outputs(picogbox)
