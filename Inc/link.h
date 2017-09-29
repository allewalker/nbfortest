#ifndef __LINK_H__
#define __LINK_H__

enum
{
	LINK_STATE_INIT,
	LINK_STATE_WAIT_PIN,
	LINK_STATE_WAIT_CGATT,
	LINK_STATE_WAIT_IP_START,
	LINK_STATE_WAIT_IP_GPRSACT,
	LINK_STATE_WAIT_IP_STATUS,
	LINK_STATE_RUN,
	LINK_STATE_WAIT_IP_DOWN,
	LINK_STATE_WAIT_LOW_POWER_MODE,
	LINK_STATE_SLEEP,
};

typedef struct
{
	uint8_t Stage;					//link状态，包括初始化，等待SIM，等待附着网络，激活APN，UDP连接，收发数据，去激活APN，进入低功耗休眠等等

}Link_CtrlStruct;
void Link_Init(void);
void Link_UrcAnalyze(int8_t *Str);
void Link_RxData(uint8_t *Data, uint16_t Len);
void Link_TxData(void);
void Link_Task(void *Param);
#endif
