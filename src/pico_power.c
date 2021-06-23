#include "picogbox.h"
#include "log.h"
#include "pico_power.h"

#include <pico.h>
#include <hardware/structs/vreg_and_chip_reset.h>
#include <hardware/vreg.h>
#include <hardware/gpio.h>
#include <pico/stdlib.h>

#include <stdio.h>

enum power_flag_e {
    POWER_FLAG_BROKEN = 0,
    POWER_FLAG_UNSTABLE = 1,
    POWER_FLAG_STABLE = 2,
    POWER_FLAG_GPIO = 4,
    POWER_FLAG_USB = 8,
    POWER_FLAG_FLASH = 16
};

enum power_test_status_e {
    POWER_TEST_STATUS_START = 64,
    POWER_TEST_STATUS_RESTART,
    POWER_TEST_STATUS_BURN,
    POWER_TEST_STATUS_BURN_RESTART,
    POWER_TEST_STATUS_BURN_DONE,
    POWER_TEST_STATUS_COMPLETE,
    POWER_TEST_STATUS_UNKNOWN
};

//
typedef struct power_level_s {
    uint8 volt;
    uint32 khz;
    uint8 flags;
} power_level_t;

// minimum rp2040 freq and voltage
static const power_level_t power_min = {
    .volt = VREG_VOLTAGE_0_90,
    .khz = 12000,
    .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH
};

// default rp2040 freq and voltage
static const power_level_t power_default = {
    .volt = VREG_VOLTAGE_1_10,
    .khz = 125000,
    .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH
};

// maximum rp2040 freq and voltage reported on forums for high quality silicon
static const power_level_t power_max = {
    .volt = VREG_VOLTAGE_1_30,
    .khz = 450000,
    .flags = POWER_FLAG_UNSTABLE
};

// power save (min volt, min freq) / balanced (default volt, max freq) / max perf (max volt, max freq) / ram mode: run stable from ram and have GPIO only
static power_level_t speed[4] = {
    { .volt = VREG_VOLTAGE_0_90, .khz = 50000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
    { .volt = VREG_VOLTAGE_1_10, .khz = 125000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
    { .volt = VREG_VOLTAGE_1_30, .khz = 250000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
    { .volt = VREG_VOLTAGE_1_30, .khz = 300000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO }
};

// test status
static uint8 test_status = POWER_TEST_STATUS_UNKNOWN;
// burn test min index
static uint8 test_index_min = 0;
// burn test max index
static uint8 test_index_max = 0;
// burn test min/max select
static uint8 test_index_select = 0;
// min and max freq per volt setting
static power_level_t power_test_level[2][9] = {
    {
        { .volt = VREG_VOLTAGE_0_90, .khz = 12000, .flags = POWER_FLAG_BROKEN },
        { .volt = VREG_VOLTAGE_0_95, .khz = 24000, .flags = POWER_FLAG_BROKEN },
        { .volt = VREG_VOLTAGE_1_00, .khz = 48000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_05, .khz = 96000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_10, .khz = 125000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_15, .khz = 125000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_20, .khz = 125000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_25, .khz = 133000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_30, .khz = 133000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH }
    },
    {
        { .volt = VREG_VOLTAGE_0_90, .khz = 48000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_0_95, .khz = 96000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_00, .khz = 125000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_05, .khz = 133000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_10, .khz = 150000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_15, .khz = 200000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_20, .khz = 250000, .flags = POWER_FLAG_STABLE && POWER_FLAG_GPIO && POWER_FLAG_USB && POWER_FLAG_FLASH },
        { .volt = VREG_VOLTAGE_1_25, .khz = 275000, .flags = POWER_FLAG_UNSTABLE },
        { .volt = VREG_VOLTAGE_1_30, .khz = 300000, .flags = POWER_FLAG_BROKEN }
    }
};
static power_level_t power_test_current = {
    .volt = VREG_VOLTAGE_0_90,
    .khz = 48000,
    .flags = 0
};

// load current values from flash
static void power_test_load(void) {
    uint8 crcsum = 0;
    // TODO load from flash and add to crcsum
    //test_status;
    //power_test_current;
    //power_test_level;
    //test_index_min;
    //test_index_max;
    //test_index_select;
    uint8 crc = 1;
    if (crcsum!=crc) {
        test_status = POWER_TEST_STATUS_UNKNOWN;
    }
}

static void power_test_store(void) {
    uint8 crcsum = 0;
    // TODO
    //store test_status;
    //store power_test_current;
    //store power_test_level;
    //store test_index_min;
    //store test_index_max;
    //store test_index_select;
    //store crc
}

static void power_test_reset(void) {
    power_test_current.volt = VREG_VOLTAGE_0_90;
    power_test_current.khz = 12000;
    power_test_current.flags = 0;
    uint8 test_index_min = 0;
    uint8 test_index_max = 0;
    uint8 test_index_select = 0;
    test_status = POWER_TEST_STATUS_START;
    power_test_store();
}

// search for levels
static void power_test_search(void) {
    // setup GPIO
    gpio_init(2);
    gpio_set_dir(2, GPIO_OUT);
    gpio_init(3);
    gpio_set_dir(3, GPIO_IN);
    printf("\nConnect GPIO2 and GPIO3 to close a loopback test circuit, then press any key.\n");
    //TODOreadLine();
    printf("\nPower test start.\n");
    // loop volts
    for (;power_test_current.volt<=power_max.volt;power_test_current.volt++) {
        printf("* testing volt=%lu\n\r",power_test_current.volt);
        vreg_set_voltage((enum vreg_voltage)power_test_current.volt);
        sleep_ms(2);
        // loop freqs, exit on completion or ... crash ...
        for(;power_test_current.khz<=power_max.khz;power_test_current.khz++) {
            printf("\t- testing freq=%lu kHz",power_test_current.khz);
            set_sys_clock_48mhz();
            sleep_ms(2);
            if (set_sys_clock_khz(power_test_current.khz, false))
            {
                sleep_ms(2);
                // test gpio
                for(int i=0; i<10; ++i)
                {
                    gpio_put(2,1); while (gpio_get(3)!=1) tight_loop_contents();
                    gpio_put(2,0); while (gpio_get(3)!=0) tight_loop_contents();
                }
                // TODO test USB
                // TODO test flash
                set_sys_clock_48mhz();
                sleep_ms(2);
                printf("works.\n\r");
                // store good status
                power_test_store();
            } else {
                printf("set clock failed.\n\r");
            }
        }
    }
    //
    printf("Power test done.\n\r");
}

static void power_test_set_level(bool min, uint8 index) {
    vreg_set_voltage((enum vreg_voltage)power_test_level[min][index].volt);
    sleep_ms(2);
    set_sys_clock_khz(power_test_level[min][index].khz, false);
}

// burn-in test to confirm levels
static void power_test_burn(void) {
}

static void power_test_select_speed(void) {
}

// read flash and (re)start test
void power_test_start(void) {
    power_test_load();
    switch (test_status) {
        case POWER_TEST_STATUS_START:
            power_test_search();
        break;
        case POWER_TEST_STATUS_RESTART:
            if (power_test_current.volt<power_max.volt) {
                power_test_current.volt++;
                power_test_search();
            } else {
                test_status = POWER_TEST_STATUS_BURN;
            }
        break;
        case POWER_TEST_STATUS_BURN:
            test_status = POWER_TEST_STATUS_RESTART;
            power_test_burn();
        break;
        case POWER_TEST_STATUS_BURN_RESTART:
            // TODO flag current as broken and test next
            power_test_burn();
        break;
        case POWER_TEST_STATUS_BURN_DONE:
            power_test_select_speed();
            test_status = POWER_TEST_STATUS_COMPLETE;
        break;
        case POWER_TEST_STATUS_COMPLETE:
            // test complete, do nothing
        break;
        case POWER_TEST_STATUS_UNKNOWN:
            power_test_reset();
            power_test_search();
        break;
    }
}

void power_set_speed(power_mode_t i) {
    if (i<=3) {
        vreg_set_voltage((enum vreg_voltage)speed[i].volt);
        sleep_ms(1);
        set_sys_clock_khz(speed[i].khz, false);
    } else {
        vreg_set_voltage((enum vreg_voltage)power_default.volt);
        sleep_ms(1);
        set_sys_clock_khz(power_default.khz, false);
    }
}

void power_set_mode(power_mode_t i) {
    switch (i) {
        case POWER_DOWN:
            // TODO disable peripherals according to requested mode
        break;
        case POWER_SAVE:
            // TODO disable peripherals according to requested mode
        break;
        case POWER_STANDBY:
            // TODO disable peripherals according to requested mode
        break;
        case POWER_STANDBY_EXTENDED:
            // TODO disable peripherals according to requested mode
        break;
        case POWER_LEVEL_SAVE:
        case POWER_LEVEL_BALANCED:
        case POWER_LEVEL_PERFORMANCE:
        case POWER_LEVEL_RAM:
        case POWER_DEFAULT:
            power_set_speed(i-POWER_OFFSET);
        break;
    }
}
