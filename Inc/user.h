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

#define APP_FLASH_START				(0x00004000)
#define BL_PARAM_START				(0x00003000)
#define APP_PARAM_START				(0x00003800)

#define __CUST_BOOTLOADER__			(0)
#define __CUST_MAIN_APP__			(1)
//#define __SYS_START_FROM_BOOTLOADER__
#define __CUST_TYPE__				__CUST_MAIN_APP__

#ifndef __SYS_START_FROM_BOOTLOADER__
#undef __CUST_TYPE__
#define __CUST_TYPE__				__CUST_MAIN_APP__
#endif

//#define __UART1_RX_DMA__
#define __UART2_RX_DMA__
//#define __UART3_RX_DMA__

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
	uint8_t USBBuf[BULK_MAX_LEN];
	RBuffer TraceBuffer;
	uint8_t TraceData[DBG_BUF_SIZE];
	uint8_t TraceDMA[DBG_BUF_SIZE];
	BL_ParamStruct BLStore;
	APP_ParamStruct AppStore;
}System_Struct;
extern System_Struct gSys;
/*******************以上内容与测试板相关**************************************************/

#endif
