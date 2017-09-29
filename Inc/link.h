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
	uint8_t Stage;					//link״̬��������ʼ�����ȴ�SIM���ȴ��������磬����APN��UDP���ӣ��շ����ݣ�ȥ����APN������͹������ߵȵ�

}Link_CtrlStruct;
void Link_Init(void);
void Link_UrcAnalyze(int8_t *Str);
void Link_RxData(uint8_t *Data, uint16_t Len);
void Link_TxData(void);
void Link_Task(void *Param);
#endif
