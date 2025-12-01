/**
 * @file ds18b20.h
 * @author A. FRAYARD
 * @brief lib for using the temperature sensor ds18b20
 * @version 1.0
 * @date 01/12/2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */



#include "main.h"
#include "usart.h"
#include "stm32l4xx_ll_usart.h"
 

/**
 * @brief function prototypes
 * 
 */

uint8_t DS18B20_Init(void);
uint8_t DS18B20_ReadBit(void);
uint8_t DS18B20_ReadByte(void);
void DS18B20_WriteByte(uint8_t);
void DS18B20_SampleTemp(void);
float DS18B20_ReadTemp(void);
void DS18B20_ListRom(void);