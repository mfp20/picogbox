#ifndef BIN_POWER_HH
#define BIN_POWER_HH

typedef enum power_mode_e {
    POWER_DOWN = 0,
    POWER_SAVE,
    POWER_STANDBY,
    POWER_STANDBY_EXTENDED,
    POWER_OFFSET,           // 4
    POWER_LEVEL_SAVE,
    POWER_LEVEL_BALANCED,
    POWER_LEVEL_PERFORMANCE,
    POWER_LEVEL_RAM,        // 8
    POWER_DEFAULT = 255
} power_mode_t;

void power_test_start(void);
void power_set_speed(power_mode_t i);
void power_set_mode(power_mode_t i);

#endif
