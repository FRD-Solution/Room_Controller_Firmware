/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "usart.h"
#include "boiler_lib.h"
#include "ow.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
#if DEBUG_TASK == true
osThreadId_t debugTaskHandle;
const osThreadAttr_t debugTask_attributes = {
  .name = "debugTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
#endif
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */
extern uint8_t FinalData[50];

extern instrument_t instrument_feeder;
extern instrument_t instrument_fan;
extern instrument_t instrument_pump;
extern instrument_t instrument_valve;
extern boiler_config_t boiler_config;

extern ow_handle_t ds18;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for feederTask */
osThreadId_t feederTaskHandle;
const osThreadAttr_t feederTask_attributes = {
  .name = "feederTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for userCommand */
osThreadId_t userCommandHandle;
const osThreadAttr_t userCommand_attributes = {
  .name = "userCommand",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for fanTask */
osThreadId_t fanTaskHandle;
const osThreadAttr_t fanTask_attributes = {
  .name = "fanTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for pumpTask */
osThreadId_t pumpTaskHandle;
const osThreadAttr_t pumpTask_attributes = {
  .name = "pumpTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for valveTask */
osThreadId_t valveTaskHandle;
const osThreadAttr_t valveTask_attributes = {
  .name = "valveTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for inletTempTask */
osThreadId_t inletTempTaskHandle;
const osThreadAttr_t inletTempTask_attributes = {
  .name = "inletTempTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for feederAutoTimer */
osTimerId_t feederAutoTimerHandle;
const osTimerAttr_t feederAutoTimer_attributes = {
  .name = "feederAutoTimer"
};
/* Definitions for usartMutex */
osMutexId_t usartMutexHandle;
const osMutexAttr_t usartMutex_attributes = {
  .name = "usartMutex"
};
/* Definitions for uartReceiveEvent */
osEventFlagsId_t uartReceiveEventHandle;
const osEventFlagsAttr_t uartReceiveEvent_attributes = {
  .name = "uartReceiveEvent"
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
#if DEBUG_TASK == true
void StartDebugTask(void *argument);
#endif

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartFeederTask(void *argument);
void StartUserCommand(void *argument);
void StartFanTask(void *argument);
void StartPumpTask(void *argument);
void StartValveTask(void *argument);
void StartInletTempTask(void *argument);
void CB_feederAutoTimer(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */
  /* Create the mutex(es) */
  /* creation of usartMutex */
  usartMutexHandle = osMutexNew(&usartMutex_attributes);

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* Create the timer(s) */
  /* creation of feederAutoTimer */
  feederAutoTimerHandle = osTimerNew(CB_feederAutoTimer, osTimerOnce, NULL, &feederAutoTimer_attributes);

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of feederTask */
  feederTaskHandle = osThreadNew(StartFeederTask, NULL, &feederTask_attributes);

  /* creation of userCommand */
  userCommandHandle = osThreadNew(StartUserCommand, NULL, &userCommand_attributes);

  /* creation of fanTask */
  fanTaskHandle = osThreadNew(StartFanTask, NULL, &fanTask_attributes);

  /* creation of pumpTask */
  pumpTaskHandle = osThreadNew(StartPumpTask, NULL, &pumpTask_attributes);

  /* creation of valveTask */
  valveTaskHandle = osThreadNew(StartValveTask, NULL, &valveTask_attributes);

  /* creation of inletTempTask */
  inletTempTaskHandle = osThreadNew(StartInletTempTask, NULL, &inletTempTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
#if DEBUG_TASK == true
  debugTaskHandle = osThreadNew(StartDebugTask, NULL, &debugTask_attributes);
#endif
  /* USER CODE END RTOS_THREADS */

  /* creation of uartReceiveEvent */
  uartReceiveEventHandle = osEventFlagsNew(&uartReceiveEvent_attributes);

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  for(;;)
  {
#if SWO == true
    printf("\n\r#####################\n\r");
    printf("WATER OUT    - %d\n\r", boiler_config.temperature_water_out);
    printf("WATER IN     - %d\n\r", boiler_config.temperature_water_in);
    printf("---------------------\n\r");
    printf("FEED ON      - %d\n\r", instrument_feeder.inst_SET);
    printf("FAN SPEED    - %d\n\r", instrument_fan.rate);
    printf("PUMP SPEED   - %d\n\r", instrument_pump.rate);
    printf("VALVE RATE   - %d\n\r", instrument_valve.rate);
    fflush(stdout);

#else
    char str[3];
    osMutexAcquire(&usartMutexHandle, 0);
    HAL_UART_Transmit(&huart1, (uint8_t*) "\n\n\rWATER OUT  - ", 17, 0xFFFF);
    HAL_UART_Transmit(&huart1, (uint8_t*) itoa(boiler_config.temperature_water_out, str, 10), 3, 0xFFFF);
    HAL_UART_Transmit(&huart1, (uint8_t*) "\n\rWATER IN   - ", 16, 0xFFFF);
    HAL_UART_Transmit(&huart1, (uint8_t*) itoa(boiler_config.temperature_water_in, str, 10), 3, 0xFFFF);
    if (instrument_feeder.status != ThreadSuspend)
    {
      HAL_UART_Transmit(&huart1, (uint8_t*) "\n\n\rFEED ON    - ", 17, 0xFFFF);
      HAL_UART_Transmit(&huart1, (uint8_t*) itoa(instrument_feeder.inst_SET, str, 10), 3, 0xFFFF);
    }
    if (instrument_fan.status != ThreadSuspend)
    {
      HAL_UART_Transmit(&huart1, (uint8_t*) "\n\rFAN SPEED  - ", 16, 10);
      HAL_UART_Transmit(&huart1, (uint8_t*) itoa(instrument_fan.rate, str, 10), 3, 0xFFFF);
    }
    if (instrument_pump.status != ThreadSuspend)
    {
      HAL_UART_Transmit(&huart1, (uint8_t*) "\n\rPUMP SPEED - ", 16, 10);
      HAL_UART_Transmit(&huart1, (uint8_t*) itoa(instrument_pump.rate, str, 10), 3, 0xFFFF);
    }
    if (instrument_valve.status != ThreadSuspend)
    {
      HAL_UART_Transmit(&huart1, (uint8_t*) "\n\rVALVE RATE - ", 16, 10);
      HAL_UART_Transmit(&huart1, (uint8_t*) itoa(instrument_valve.rate, str, 10), 3, 0xFFFF);
    }
    osMutexRelease(&usartMutexHandle);
#endif
    osDelay(10000);
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_StartFeederTask */
/**
* @brief Function implementing the feederTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFeederTask */
void StartFeederTask(void *argument)
{
  /* USER CODE BEGIN StartFeederTask */
  /* Infinite loop */
  osTimerStart(feederAutoTimerHandle, 1000);
  for(;;)
  {
    feeder_cruise();
  }
  /* USER CODE END StartFeederTask */
}

/* USER CODE BEGIN Header_StartUserCommand */
/**
* @brief Function implementing the userCommand thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUserCommand */
void StartUserCommand(void *argument)
{
  /* USER CODE BEGIN StartUserCommand */
  /* Infinite loop */
  for(;;)
  {
    if(osEventFlagsGet(uartReceiveEventHandle))
    {
      osEventFlagsClear(uartReceiveEventHandle, 1);
      command_selection(FinalData);
    }
    osDelay(100);
  }
  /* USER CODE END StartUserCommand */
}

/* USER CODE BEGIN Header_StartFanTask */
/**
* @brief Function implementing the fanTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartFanTask */
void StartFanTask(void *argument)
{
  /* USER CODE BEGIN StartFanTask */
  /* Infinite loop */
  for(;;)
  {
    fan_cruise();
  }
  /* USER CODE END StartFanTask */
}

/* USER CODE BEGIN Header_StartPumpTask */
/**
* @brief Function implementing the pumpTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartPumpTask */
void StartPumpTask(void *argument)
{
  /* USER CODE BEGIN StartPumpTask */
  /* Infinite loop */
  for(;;)
  {
    pump_cruise();
  }
  /* USER CODE END StartPumpTask */
}

/* USER CODE BEGIN Header_StartValveTask */
/**
* @brief Function implementing the valveTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartValveTask */
void StartValveTask(void *argument)
{
  /* USER CODE BEGIN StartValveTask */
  /* Infinite loop */
  for(;;)
  {
    valve_cruise();
  }
  /* USER CODE END StartValveTask */
}

/* USER CODE BEGIN Header_StartInletTempTask */
/**
* @brief Function implementing the inletTempTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartInletTempTask */
void StartInletTempTask(void *argument)
{
  /* USER CODE BEGIN StartInletTempTask */
  /* Infinite loop */
  for(;;)
  {
    uint8_t data[16];
    ow_update_rom_id(&ds18);
    //while (ow_is_busy(&ds18));
    osDelay(10);
    ow_xfer(&ds18, 0x44, NULL, 0, 0);
    osDelay(1000);
    ow_xfer(&ds18, 0xBE, NULL, 0, 9);
    //while (ow_is_busy(&ds18));
    ow_read_resp(&ds18, data, 16);

    osDelay(1000);
  }
  /* USER CODE END StartInletTempTask */
}

/* CB_feederAutoTimer function */
void CB_feederAutoTimer(void *argument)
{
  /* USER CODE BEGIN CB_feederAutoTimer */
  if (boiler_config.status != ErrorWaterHot)
  {
    if ((instrument_feeder.status == feedON) || (instrument_feeder.status == OK))
      {
        instrument_feeder.status = feedOFF;
        HAL_GPIO_WritePin(GPIO_Feeder_Output_GPIO_Port, GPIO_Feeder_Output_Pin, RESET);
        HAL_GPIO_WritePin(GPIO_LED_BUILTIN_GPIO_Port, GPIO_LED_BUILTIN_Pin, RESET);
        if (instrument_feeder.inst_RESET)
        {
          osTimerStart(feederAutoTimerHandle, instrument_feeder.inst_RESET*1000);
        }
        else
        {
          osTimerStart(feederAutoTimerHandle, 5000);
        }
      }
    else if (instrument_feeder.status == feedOFF)
    {
      instrument_feeder.status = feedON;
      HAL_GPIO_WritePin(GPIO_Feeder_Output_GPIO_Port, GPIO_Feeder_Output_Pin, SET);
      HAL_GPIO_WritePin(GPIO_LED_BUILTIN_GPIO_Port, GPIO_LED_BUILTIN_Pin, SET);
      if (instrument_feeder.inst_SET){
        osTimerStart(feederAutoTimerHandle, instrument_feeder.inst_SET*1000);
      }
      else
      {
          osTimerStart(feederAutoTimerHandle, 5000);
      }
    }
  }
  else
  {
    instrument_feeder.status = feedOFF;
    HAL_GPIO_WritePin(GPIO_Feeder_Output_GPIO_Port, GPIO_Feeder_Output_Pin, RESET);
    osTimerStart(feederAutoTimerHandle, 10000);
  }
  /* USER CODE END CB_feederAutoTimer */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void StartDebugTask(void *argument)
{
  uint8_t ramp_up = 1;
  for (;;)
  {
    if (ramp_up == 1)
    {
      boiler_config.temperature_water_in++;
      boiler_config.temperature_water_out = boiler_config.temperature_water_in + 5;
    }
    else
    {
      boiler_config.temperature_water_in--;
      boiler_config.temperature_water_out = boiler_config.temperature_water_in + 5;
    }

    switch (boiler_config.temperature_water_in)
    {
    case 85:
      ramp_up = 0;
      break;
    
    case -15:
      ramp_up = 1;
      break;
    }
    osDelay(10000);
  }
  
}
/* USER CODE END Application */

