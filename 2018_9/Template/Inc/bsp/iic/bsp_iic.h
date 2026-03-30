#include "main.h"

void I2CStart(void);
void I2CStop(void);
unsigned char I2CWaitAck(void);
void I2CSendAck(void);
void I2CSendNotAck(void);
void I2CSendByte(unsigned char cSendByte);
unsigned char I2CReceiveByte(void);
void I2CInit(void);
void AT24C02_Write(uint8_t addr,uint8_t *ptr,uint8_t Num);
void AT24C02_Read(uint8_t addr,uint8_t *ptr,uint8_t Num);
void MCP1407_Write(uint8_t Res);
uint8_t MCP1407_Read(void);

