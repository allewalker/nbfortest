#ifndef __IO_H__
#define __IO_H__

enum
{
	PIN_IOT_POWER,
	PIN_IOT_READY,
	PIN_IOT_SLEEP,
	PIN_STATE,
	PIN_MAX,
};

void IO_ConfigAll(void);
void IO_ConfigExit(uint8_t Sn, MyAPIFunc ISR, uint8_t IRQn, uint8_t IRQLevel);
void IO_Write(uint8_t Sn, uint8_t Val);
uint8_t IO_Read(uint8_t Sn);
#endif
