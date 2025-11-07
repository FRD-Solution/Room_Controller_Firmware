/**
 * @file boiler_lib.c
 * @author A. FRAYARD (antoine.frayard@idemoov.fr)
 * @brief 
 * @version 1.0
 * @date 03/11/2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */


/* Global import*/
#include <string.h>
#include <stdlib.h>

/* Local Import*/
#include "boiler_lib.h"
#include "cmsis_os.h"
#include "cmsis_os2.h"
#include "usart.h"
#include "tim.h"



/**
 * @brief extern variable declaration
 */

extern osMutexId_t usartMutexHandle;
extern osThreadId_t feederTaskHandle;
extern osThreadId_t fanTaskHandle;
extern osThreadId_t pumpTaskHandle;
extern osThreadId_t valveTaskHandle;

extern osTimerId_t feederAutoTimerHandle;

instrument_t instrument_feeder = {
    .inst_SET = 5,      //amount of time in sec the motor is turning
    .inst_RESET = 100,   //amount of time in sec the motor is resting
    .status = feedOFF,
    .rate = -1,         // motor angular speed
    .sensor_status = sensorNOK,
};

instrument_t instrument_fan = {
    .rate = 50, // PWM speed modulation (duty cycle between 0-100%)
    .status = OK,
    .sensor_status = sensorNOK,
};
instrument_t instrument_pump = {
    .rate = 0,  // selected speed mode (1, 2 or 3)
    .status = OK,
    .sensor_status = sensorNOK,
};

instrument_t instrument_valve = {
    .rate = 90,  // angular position between 0-90 (0->loopback water out to water in)
    .status = OK,
    .sensor_status = sensorNOK,
};

boiler_config_t boiler_config = {
    .temperature_water_in = 15,
    .temperature_water_out = 15,
    .sensor_status = sensorNOK,
    .mode = AUTOMATIC_MODE,
    .status = OK,
};



void feeder_cruise(void)
{
    if (boiler_config.temperature_water_out < 80)
    {
        boiler_config.status = OK;
        if (boiler_config.mode == AUTOMATIC_MODE)
        {
            //instrument_feeder.status = OK;
            instrument_feeder.inst_SET = ((80 - boiler_config.temperature_water_out) / 2) / (100/instrument_feeder.inst_RESET);
        }
    }
    else
    {
        instrument_feeder.inst_SET = 0;
        boiler_config.status = ErrorWaterHot;
    }
    if (instrument_feeder.sensor_status == Debug)
    {
        if (instrument_feeder.status == feedON)
        {
            osMutexAcquire(usartMutexHandle, 0);
            HAL_UART_Transmit(&huart1,(uint8_t*) "\n\rDEBUG - FEEDER ON", 20, 0xFFFF);
            osMutexRelease(usartMutexHandle);
        }
        else if (instrument_feeder.status == feedOFF)
        {
            osMutexAcquire(usartMutexHandle, 0);
            HAL_UART_Transmit(&huart1,(uint8_t*) "\n\rDEBUG - FEEDER OFF", 21, 0xFFFF);
            osMutexRelease(usartMutexHandle);
        }
        else
        {
            osMutexAcquire(usartMutexHandle, 0xffff);
            HAL_UART_Transmit(&huart1,(uint8_t*) "\n\rDEBUG - FEEDER NOK", 21, 0xFFFF);
            osMutexRelease(usartMutexHandle);
        }
    }
    osDelay(1000);
}

void fan_cruise(void)
{
    if ((boiler_config.temperature_water_out < 50))
    {
        boiler_config.status = OK;
        if (boiler_config.mode == AUTOMATIC_MODE)
        {
            instrument_fan.status = fanON;
            instrument_fan.rate = 80;
        }
    }
    else if ((boiler_config.temperature_water_out < 62))
    {
        boiler_config.status = OK;
        if (boiler_config.mode == AUTOMATIC_MODE)
        {
            instrument_fan.status = fanON;
            instrument_fan.rate = 80 + (((62 - boiler_config.temperature_water_out) -12) * 5);
        }
    }
    else if ((boiler_config.temperature_water_out < 80))
    {
        boiler_config.status = OK;
        if (boiler_config.mode == AUTOMATIC_MODE)
        {
            instrument_fan.status = fanON;
            instrument_fan.rate = 20;
        }
    }
    else
    {
        instrument_fan.status = fanOFF;
        instrument_fan.rate = 0;
        boiler_config.status = ErrorWaterHot;
    } 
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, instrument_fan.rate);
    osDelay(1000);
}

void pump_cruise(void)
{
    if (boiler_config.temperature_water_out < 80)
    {
        boiler_config.status = OK;
        if (boiler_config.mode == AUTOMATIC_MODE)
        {
            if (boiler_config.temperature_water_out<60)
            {
                instrument_pump.rate = 0;
            }
            else if (boiler_config.temperature_water_out<70)
            {
                instrument_pump.rate = 1;
            }
            else if (boiler_config.temperature_water_out<80)
            {
                instrument_pump.rate = 2;
            }
            else
            {
                instrument_pump.rate = 3;
            }
        }
    }
    else
    {
        boiler_config.status = ErrorWaterHot;
        instrument_pump.rate = 3;
    }
    HAL_GPIO_WritePin(GPIO_Pump_Output_GPIO_Port, GPIO_Pump_Output_Pin, instrument_pump.rate);
    osDelay(1000);
}

void valve_cruise(void)
{
    if(boiler_config.mode == AUTOMATIC_MODE)
    {
        if ((boiler_config.temperature_water_out >= 80) || (boiler_config.temperature_water_in >= 80))
        {
            boiler_config.status = ErrorWaterHot;
            set_valve_position(90);
        }
        else if (boiler_config.temperature_water_in < 62)
        {
            boiler_config.status = OK;
            set_valve_position(0);
        }
        else
        {
            boiler_config.status = OK;
            set_valve_position((boiler_config.temperature_water_in - 62) *5);
        }
    }
    osDelay(1000);
}

void set_valve_position(uint8_t new_angle)
{
    if (instrument_valve.rate < new_angle)
    {
        HAL_GPIO_WritePin(GPIO_Valve_Output_2_GPIO_Port, GPIO_Valve_Output_2_Pin, RESET);
        HAL_GPIO_WritePin(GPIO_Valve_Output_1_GPIO_Port, GPIO_Valve_Output_1_Pin, SET);
        osDelay((new_angle - instrument_valve.rate)*100);
        HAL_GPIO_WritePin(GPIO_Valve_Output_1_GPIO_Port, GPIO_Valve_Output_1_Pin, RESET);
    }
    else if(instrument_valve.rate > new_angle)
    {
        HAL_GPIO_WritePin(GPIO_Valve_Output_1_GPIO_Port, GPIO_Valve_Output_1_Pin, RESET);
        HAL_GPIO_WritePin(GPIO_Valve_Output_2_GPIO_Port, GPIO_Valve_Output_2_Pin, SET);
        osDelay((instrument_valve.rate - new_angle)*100);
        HAL_GPIO_WritePin(GPIO_Valve_Output_2_GPIO_Port, GPIO_Valve_Output_2_Pin, RESET);
    }
    instrument_valve.rate = new_angle;
}


void command_selection(uint8_t *command_string)
{
    char* cmd_instrument = strtok((char*) command_string, (char*) "</");
    char* cmd_action = strtok(NULL, (char*) "</");
    char* cmd_data = strtok(NULL, (char*) "</");
    instrument_t *selected_instrument;
    boiler_config_t *selected_boiler;
    osThreadId_t selected_instrument_thread_id;


    /**
     * @brief retrieving the instrument from the user command and pre-selecting it
     */
    if(strcmp(cmd_instrument, CMD_INSTRUMENT_FEED) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "COMMAND FEEDER ", 16, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument = &instrument_feeder;
        selected_instrument_thread_id = feederTaskHandle;
    }
    else if(strcmp(cmd_instrument, CMD_INSTRUMENT_FAN) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "COMMAND FAN ", 13, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument = &instrument_fan;
        selected_instrument_thread_id = fanTaskHandle;
    }
    else if(strcmp(cmd_instrument, CMD_INSTRUMENT_PUMP) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "COMMAND PUMP ", 14, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument = &instrument_pump;
        selected_instrument_thread_id = pumpTaskHandle;
    }
    else if(strcmp(cmd_instrument, CMD_INSTRUMENT_VALVE) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "COMMAND VALVE ", 15, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument = &instrument_valve;
        selected_instrument_thread_id = valveTaskHandle;
    }
    else if(strcmp(cmd_instrument, CMD_BOILER_CONFIG) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "COMMAND BOILER ", 16, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_boiler = &boiler_config;
    }

    /**
     * @brief retrieving the action from the user command and executing it for the pre-selected instrument
     * 
     */
    if(strcmp(cmd_action, CMD_ACTION_SET) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- SET\n\r", 8, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument->inst_SET = strtoul((char*)cmd_data, (char**) NULL, 10);
    }
    else if(strcmp(cmd_action, CMD_ACTION_RESET) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- RESET\n\r", 8, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument->inst_RESET = strtoul((char*)cmd_data, (char**) NULL, 10);
    }
    else if(strcmp(cmd_action, CMD_ACTION_SPEED) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- SPEED\n\r", 8, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument->rate = strtoul((char*)cmd_data, (char**) NULL, 10);
    }
    else if(strcmp(cmd_action, CMD_ACTION_DEBUG) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- DEBUG\n\r", 8, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_instrument->sensor_status = Debug;
    }
    else if(strcmp(cmd_action, CMD_ACTION_START) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- START\n\r", 10, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        osThreadResume(selected_instrument_thread_id);
        selected_instrument->status = OK;
    }
    else if(strcmp(cmd_action, CMD_ACTION_STOP) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- STOP\n\r", 9, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        osThreadSuspend(selected_instrument_thread_id);
        selected_instrument->status = ThreadSuspend;
    }
    else if(strcmp(cmd_action, CMD_ACTION_WATER_OUT) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- WATER OUT\n\r", 14, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_boiler->temperature_water_out = strtoul((char*)cmd_data, (char**) NULL, 10);
    }
    else if(strcmp(cmd_action, CMD_ACTION_WATER_IN) == 0)
    {
        osMutexAcquire(&usartMutexHandle, 0);
        HAL_UART_Transmit(&huart1, (uint8_t*) "- WATER IN\n\r", 13, 0xFFFF);
        osMutexRelease(&usartMutexHandle);
        selected_boiler->temperature_water_in = strtoul((char*)cmd_data, (char**) NULL, 10);
    }
}