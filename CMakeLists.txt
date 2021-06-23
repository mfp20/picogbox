cmake_minimum_required(VERSION 3.12)
include(pico_sdk_import.cmake)
project(picogbox)
pico_sdk_init()
add_definitions(-DPICO)

if ("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
        add_definitions(-DBUILD_DEBUG)
else()
        add_definitions(-DBUILD_RELEASE)
endif()

# slow down flash in order to reach higher overclocks
#pico_define_boot_stage2(slower_boot2 ${PICO_DEFAULT_BOOT_STAGE2})
#target_compile_definitions(slower_boot2 PRIVATE PICO_FLASH_SPI_CLKDIV=4)
#pico_set_boot_stage2(TARGET slower_boot2)

add_executable(picogbox
        src/utils.c
        src/log.c
        src/manager.c
        src/managed_uart.c
        src/pico_led.c
        src/pico_serialid.c
        src/pico_power.c
        src/tusb_config.c
        src/bin_cdc_microshell.c
        src/bin_cdc_binary.c
        src/bin_cdc_bridge_uart.c
        src/bin_cdc_sump.c
        src/bin_cdc_capture.c
        src/bin_vendor_swd.c
        src/picogbox.c
)

target_include_directories(picogbox PRIVATE 
                        src
                        )

target_link_libraries(picogbox PRIVATE
                        cmsis_core
                        pico_stdlib
                        pico_stdio_uart
                        pico_unique_id
                        pico_multicore
                        tinyusb_device 
                        tinyusb_board
                        hardware_regs
                        hardware_structs
                        hardware_vreg
                        hardware_timer
                        hardware_rtc
                        hardware_pio 
                        hardware_dma 
                        hardware_pwm
                        hardware_uart
                        hardware_i2c
                        hardware_spi
                        ush
                        )

include(src/microshell/CMakeLists.txt)
build_ush(ush)

if (DEFINED ENV{TURBO_200MHZ})
        message("TURBO_200MHZ is defined as " $ENV{TURBO_200MHZ})
        target_compile_definitions(picogbox PRIVATE TURBO_200MHZ=$ENV{TURBO_200MHZ})
endif()

pico_enable_stdio_uart(picogbox 0)
pico_enable_stdio_usb(picogbox 0)
pico_generate_pio_header(picogbox ${CMAKE_CURRENT_LIST_DIR}/src/bin_vendor_swd.pio)
pico_add_extra_outputs(picogbox)
