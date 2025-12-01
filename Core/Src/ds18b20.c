/**
 * @file ds18b20.c
 * @author A. FRAYARD 
 * @brief lib for using the temperature sensor ds18b20
 * @version 1.0
 * @date 01/12/2025
 * 
 * @copyright Copyright (c) 2025
 * 
 */

#include "ds18b20.h"









uint8_t DS18B20_Init(void)
{
    uint8_t ResetByte = 0xF0, PresenceByte;
    LL_USART_SetBaudRate(huart4.Instance, HAL_RCC_GetPCLK2Freq(), UART_OVERSAMPLING_16, 9600);
    // Send reset pulse (0xF0)
    // HAL_HalfDuplex_EnableTransmitter(&huart4);
    HAL_UART_Transmit(&huart4, &ResetByte, 1, 0xffff);
    // Wait for the presence pulse
    // HAL_HalfDuplex_EnableReceiver(&huart4);
    HAL_UART_Receive(&huart4, &PresenceByte, 1, 1);
    LL_USART_SetBaudRate(huart4.Instance, HAL_RCC_GetPCLK2Freq(), UART_OVERSAMPLING_16, 115200);
    // Check presence pulse
    if (PresenceByte != ResetByte){
        return 1; // Presence pulse detected
    }
    else{
        return 0; // No presence pulse detected
    }
}
 
uint8_t DS18B20_ReadBit(void)
{
    uint8_t ReadBitCMD = 0xFF;
    uint8_t RxBit;
 
    // Send Read Bit CMD
    // HAL_HalfDuplex_EnableTransmitter(&huart4);
    HAL_UART_Transmit(&huart4, &ReadBitCMD, 1, 0xffff);
    // Receive The Bit
    // HAL_HalfDuplex_EnableReceiver(&huart4);
    HAL_UART_Receive(&huart4, &RxBit, 1, 1);
 
    return (RxBit & 0x01);
}
 
uint8_t DS18B20_ReadByte(void)
{
    uint8_t RxByte = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        RxByte >>= 1;
        if (DS18B20_ReadBit())
        {
            RxByte |= 0x80; // because 1 bit on uint8_t = 1000 0000
        }
    }
    return RxByte;
}
 
void DS18B20_WriteByte(uint8_t data)
{
    uint8_t TxBuffer[8];
    for (int i=0; i<8; i++)
    {
      if ((data & (1<<i)) != 0){
          TxBuffer[i] = 0xFF;
      }
      else{
          TxBuffer[i] = 0;
      }
    }
    // HAL_HalfDuplex_EnableTransmitter(&huart4);
    HAL_UART_Transmit(&huart4, TxBuffer, 8, 0xffff);
}
 
void DS18B20_SampleTemp(void)
{
    DS18B20_Init();
    DS18B20_WriteByte(0xCC);  // Skip ROM   (ROM-CMD)
    DS18B20_WriteByte(0x44);  // Convert T  (F-CMD)
}
 
float DS18B20_ReadTemp(void)
{
    uint8_t Temp_LSB, Temp_MSB;
    uint16_t Temp;
    float Temperature;
 
    DS18B20_Init();
    DS18B20_WriteByte(0xCC);  // Skip ROM         (ROM-CMD)
    DS18B20_WriteByte(0xBE);  // Read Scratchpad  (F-CMD)
    Temp_LSB = DS18B20_ReadByte();
    Temp_MSB = DS18B20_ReadByte();
    Temp = ((Temp_MSB<<8))|Temp_LSB;
    Temperature = (float)Temp/16.0;
 
    return Temperature;
}

void DS18B20_ListRom(void)
{
    uint8_t rom_addr1[8];
    uint8_t rom_addr2[8];
    DS18B20_Init();
    DS18B20_WriteByte(0xF0); // Search Rom
    for (size_t i = 0; i < 8; i++)
    {
        rom_addr1[i] = DS18B20_ReadByte();
    }
    DS18B20_WriteByte(0xF0); // Search Rom
    for (size_t i = 0; i < 8; i++)
    {
        rom_addr2[i] = DS18B20_ReadByte();
    }
    return; 
}