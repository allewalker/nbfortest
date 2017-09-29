#ifndef __USER_H__
#define __USER_H__


#ifndef PI
#define PI					3.14159265358979f
#endif

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include "config.h"
#include "stm32f10x_conf.h"
#include "CApi.h"
#include "nvram.h"
#include "debug.h"
#include "setting.h"
#include "timer.h"
#include "usart.h"
#include "link.h"
#include "at.h"


enum VAR_ENUM
{
	CHIP_ID0,
	CHIP_ID1,
	CHIP_ID2,
	FLASH_SIZE,
	VERSION,
	SYS_FRQ,
	UTC_DATE,
	UTC_TIME,
	SYS_VAR_MAX,
};

typedef struct
{
	uint32_t CRC32_Table[256];
	uint32_t Var[SYS_VAR_MAX];
	uint32_t LastDay;
	Timer_CtrlStruct TimerList[TIMER_MAX];
	Uart_CtrlStruct Uart[UART_MAX];
	//uint8_t USBBuf[BULK_MAX_LEN];
	RBuffer TraceBuffer;
	uint8_t TraceData[DBG_BUF_SIZE];
	uint8_t TraceDMA[DBG_BUF_SIZE];
#ifdef __SYS_START_FROM_BOOTLOADER__
	BL_ParamStruct BLStore;
	APP_ParamStruct AppStore;
#endif
}System_Struct;
extern System_Struct gSys;
/*******************以上内容与测试板相关**************************************************/

#endif
