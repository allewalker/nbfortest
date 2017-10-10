#ifndef __LINK_H__
#define __LINK_H__

#define SOCKET_MAX	(6)

enum
{
	LINK_MAIN_STATE_INIT,
	LINK_MAIN_STATE_ACTIVE,
	LINK_MAIN_STATE_COMM,
	LINK_MAIN_STATE_SLEEP,

	LINK_SUB_STATE_POWER_DOWN,
	LINK_SUB_STATE_POWER_ON,
	LINK_SUB_STATE_WAIT_READY,
	LINK_SUB_STATE_INIT_QUEUE,
	LINK_SUB_STATE_WAIT_PIN,
	LINK_SUB_STATE_WAIT_CGATT,
	LINK_SUB_STATE_WAIT_IP_START,
	LINK_SUB_STATE_WAIT_IP_GPRSACT,
	LINK_SUB_STATE_WAIT_IP_STATUS,
	LINK_SUB_STATE_RUN_IDLE,
	LINK_SUB_STATE_CONNECT,
	LINK_SUB_STATE_TX,
	LINK_SUB_STATE_CLOSE,
	LINK_SUB_STATE_WAIT_IP_DOWN,

	LINK_EVENT_CONNECT_OK,
	LINK_EVENT_CONNECT_FAIL,
	LINK_EVENT_TX_OK,
	LINK_EVENT_TX_FAIL,
	LINK_EVENT_RX,
	LINK_EVENT_CLOSE,

	LINK_REQ_CONNECT,
	LINK_REQ_TX,
	LINK_REQ_CLOSE,

	LINK_IP_INITIAL = 0,
	LINK_IP_START,
	LINK_IP_CONFIG,
	LINK_IP_GPRSACT,
	LINK_IP_STATUS,
	LINK_IP_PROCESSING,
	LINK_IP_PDP_DEACT,
};



typedef struct
{
	void*			RxData;
	uint16_t   		RxDataLen;
	int8_t    		SocketID;    /* socket ID */
	uint8_t 		Event;   /* soc_event_enum */
}Link_NotifyStruct;

typedef struct
{
	void *Data;
	uint16_t Len;
	uint8_t SocketID;
	uint8_t Req;
}Link_ReqStruct;

typedef struct
{
	Link_ReqStruct ReqBuf[SOCKET_MAX];
	RBuffer ReqList;
	void *TxBuf;
	uint16_t TxLen;
	uint8_t IPState;
	uint32_t RxSocketID;
	uint32_t RxLen;
	MyCBFun_t NotifyCB[SOCKET_MAX];
	uint8_t SocketState[SOCKET_MAX];
	uint8_t MainState;					//link״̬��������ʼ�����������磬ͨѶ������
	uint8_t SubState;
	uint8_t ToFlag;
	int8_t TxSocketID;
}Link_CtrlStruct;
void Link_Init(void);
void Link_UrcAnalyze(int8_t *Str);
void Link_RxData(uint8_t *Data, uint16_t Len);
void Link_TxData(void);
void Link_Task(void *Param);
void Link_Restart(void);
#endif
