/**
 * @file boiler_lib.h
 * @author A. FRAYARD (antoine.frayard@idemoov.fr)
 * @brief 
 * @version 1.0
 * @date 03/11/2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "main.h"

#define CMD_INSTRUMENT_FEED     "feed"
#define CMD_INSTRUMENT_FAN      "fan"
#define CMD_INSTRUMENT_PUMP     "pump"
#define CMD_INSTRUMENT_VALVE    "valve"
#define CMD_BOILER_CONFIG       "boiler"

#define CMD_ACTION_SET          "set"
#define CMD_ACTION_RESET        "reset"
#define CMD_ACTION_SPEED        "speed"
#define CMD_ACTION_WATER_OUT    "waterout"
#define CMD_ACTION_WATER_IN     "waterin"
#define CMD_ACTION_START        "start"
#define CMD_ACTION_STOP         "stop"
#define CMD_ACTION_DEBUG        "debug"

#define MANUAL_MODE             0
#define AUTOMATIC_MODE          1


typedef enum {
  OK = 0,
  feedON,                       // feeder motor is turning
  feedOFF,                      // feeder motor is stopped (e.g. fuel tank open)
  fanON,
  fanOFF,
  sensorOK,
  sensorNOK,
  ThreadSuspend,
  ErrorThermalProtection,       // thermal protection contact is ON (feeder motor is probably stuck)
  ErrorStuck,                   // feeder motor didn't moved in the specified timeout (motor is stuck)
  ErrorWaterHot,
  ErrorWaterCold,
  Debug,
}boiler_status_t;


typedef struct {
    uint8_t inst_SET;     // amount of time (in sec) the feeder must turn per cycle
    uint8_t inst_RESET;    // amount of time (in sec) the feeder must wait per cycle
    boiler_status_t status; // feeder info status
    int8_t rate;          // feeder measured turning speed (-1 if not measured)
    boiler_status_t sensor_status;
}instrument_t;

typedef struct {
  int16_t temperature_water_in;
  int16_t temperature_water_out;
  boiler_status_t sensor_status;
  boiler_status_t status;
  uint8_t mode;
}boiler_config_t;

/**
 * @brief function prototypes
 */

void feeder_cruise(void);
void fan_cruise(void);
void pump_cruise(void);
void valve_cruise(void);
void command_selection(uint8_t* command_string);

void set_valve_position(uint8_t new_angle);




