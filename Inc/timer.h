#ifndef __TIMER_H__
#define __TIMER_H__
#include "CApi.h"
typedef struct
{
	uint32_t LastTime;
	uint32_t ReloadTime;
	MyCBFun_t CallBack;
	uint32_t Param;
	uint8_t Work;
	uint8_t ToFlag;
	uint8_t pad[2];
}Timer_CtrlStruct;

/*********************************TimerList*****************************************/
enum
{
	CORE_TIMER,
	MODBUS_TIMER,
	TIMER_MAX
};
void Timer_Config(void);
void Timer_Task(void);
int Timer_Start(uint8_t ID, uint32_t To, uint8_t IsRepeat, MyCBFun_t CB);
void Timer_Switch(uint8_t ID, uint8_t OnOff);
void Timer_Del(uint8_t ID);
uint8_t Timer_Check(uint8_t ID);
#endif
