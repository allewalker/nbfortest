#ifndef __LINK_H__
#define __LINK_H__
#define SOCKET_MAX	(6)
//#define __NB_SIMCOM7000C__
#define __NB_BC95__
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
	LINK_SUB_STATE_WAIT_INIT_DONE,
	LINK_SUB_STATE_WAIT_PIN,
	LINK_SUB_STATE_WAIT_CGATT,
	LINK_SUB_STATE_WAIT_IP_OK,
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

#ifdef __NB_SIMCOM7000C__
#define AT_BR		(115200)
typedef struct
{
	Link_ReqStruct ReqBuf[SOCKET_MAX];
	RBuffer ReqList;
	Link_ReqStruct CurReq;
	void *TxBuf;
	uint16_t TxLen;
	uint8_t IPState;
	uint32_t RxSocketID;
	uint32_t RxLen;
	MyCBFun_t NotifyCB[SOCKET_MAX];
	uint8_t MainState;					//link状态，包括初始化，激活网络，通讯，休眠
	uint8_t SubState;
	uint8_t ToFlag;
	uint8_t PinCheck;
	uint8_t AttachCheck;
	uint8_t ActiveCheck;
	int8_t TxSocketID;
}Link_CtrlStruct;
#endif

#ifdef __NB_BC95__
#define AT_BR		(9600)
typedef struct
{
	Link_ReqStruct ReqBuf[SOCKET_MAX];
	RBuffer ReqList;
	Link_ReqStruct CurReq;
	uint8_t TxBuf[1200];			//华为特殊的AT指令
	uint8_t RxBuf[512];				//华为特殊的AT指令
	MyCBFun_t NotifyCB[SOCKET_MAX];

	int8_t *IP[SOCKET_MAX];
	uint16_t Port[SOCKET_MAX];
	uint8_t MainState;					//link状态，包括初始化，激活网络，通讯，休眠
	uint8_t SubState;
	uint8_t ToFlag;
	uint8_t AttachCheck;
	int8_t TxSocketID;
	uint8_t SocketBit;
}Link_CtrlStruct;
#endif
void Link_Init(void);
void Link_UrcAnalyze(int8_t *Str);
void Link_RxData(uint8_t *Data, uint16_t Len);
void Link_TxData(void);
void Link_Task(void *Param);
void Link_Restart(void);
void Link_RegSocket(uint8_t SocketID, MyCBFun_t CB);
void Link_AddReq(uint8_t SocketID, uint8_t ReqType, void *Data, uint16_t Len);
#endif
