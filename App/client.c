#include "user.h"
char* IP = "115.231.73.229";
#define CONNECT_TO	(120)
#define PORT (9888)
#define UART_TO	(50)
#define LTCC_DATA_LEN_MAX	264
enum
{
	SOCKET_STATE_OFFLINE,
	SOCKET_STATE_CONNECT,
	SOCKET_STATE_ONLINE,
	SOCKET_STATE_WAIT_TX,
	SOCKET_STATE_WAIT_RX,
};

typedef struct
{
	uint8_t Data[LTCC_DATA_LEN_MAX];
	uint32_t Len;
}LTCC_CacheStruct;
static uint32_t PackID;
static uint8_t TxBuf[1460];
static uint16_t TxLen;
static LTCC_CacheStruct UartRx;
static uint8_t SocketState;
static LTCC_CacheStruct Cache[16];
static RBuffer CacheBuf;
static uint8_t ReSendFlag;
static void Client_NotifyCB(void *pData)
{
	Link_NotifyStruct *Notify = (Link_NotifyStruct *)pData;
	int8_t *Str;
	switch (Notify->Event)
	{
	case LINK_EVENT_RX:

		Str = (int8_t*)Notify->RxData;
		Str[Notify->RxDataLen] = 0;
		DBG_INFO("%d,%s", Notify->RxDataLen, Str);
		if (SocketState == SOCKET_STATE_WAIT_RX)
		{
			SocketState = SOCKET_STATE_ONLINE;
		}
		break;
	case LINK_EVENT_TX_OK:
		if (SocketState == SOCKET_STATE_WAIT_TX)
		{
			SocketState = SOCKET_STATE_ONLINE;
		}
		break;
	case LINK_EVENT_CONNECT_OK:
		if (SocketState == SOCKET_STATE_CONNECT)
		{
			SocketState = SOCKET_STATE_ONLINE;
		}
		break;
	case LINK_EVENT_CLOSE:
		SocketState = SOCKET_STATE_OFFLINE;
		break;
	case LINK_EVENT_CONNECT_FAIL:
		SocketState = SOCKET_STATE_OFFLINE;
		break;
	case LINK_EVENT_TX_FAIL:
		DBGF;
		Link_Restart();
		SocketState = SOCKET_STATE_OFFLINE;
		ReSendFlag = 1;
		//UDP模式下不需要
//		Link_AddReq(SOCKET_CLIENT_ID, LINK_REQ_CLOSE, IP, PORT);
//		SocketState = SOCKET_STATE_OFFLINE;
		break;
	}
}

static void Client_TOCB(void *pData)
{
	WriteRBufferForce(&CacheBuf, &UartRx, 1);
	UartRx.Len = 0;
	Timer_Switch(CLIENT_TIMER, 0);
}

static void Client_UartCB(void *pData)
{
	uint32_t Type = (uint32_t)pData;
	uint8_t Temp;
	if (Type == UART_RX_IRQ)
	{
		Temp = Uart_Rx(DTU_UART_ID);
		UartRx.Data[UartRx.Len] = Temp;
		UartRx.Len++;
		if (UartRx.Len >= LTCC_DATA_LEN_MAX)
		{
			WriteRBufferForce(&CacheBuf, &UartRx, 1);
			UartRx.Len = 0;
		}
		Timer_Restart(CLIENT_TIMER, UART_TO);
	}
}

void Client_Init(void)
{
	Link_RegSocket(SOCKET_CLIENT_ID, Client_NotifyCB);
	SocketState = SOCKET_STATE_OFFLINE;
	InitRBuffer(&CacheBuf, Cache, sizeof(Cache)/sizeof(LTCC_CacheStruct), sizeof(LTCC_CacheStruct));
	Uart_Config(DTU_UART_ID, DTU_BR, Client_UartCB, 1);
	Timer_Start(CLIENT_TIMER, 50, 0, Client_TOCB);
	Timer_Switch(CLIENT_TIMER, 0);
}

void Client_Task(void *pData)
{
	LTCC_CacheStruct *Temp;
	switch (SocketState)
	{
	case SOCKET_STATE_OFFLINE:
		Link_AddReq(SOCKET_CLIENT_ID, LINK_REQ_CONNECT, IP, PORT);
		SocketState = SOCKET_STATE_CONNECT;
		break;
	case SOCKET_STATE_CONNECT:
		break;
	case SOCKET_STATE_ONLINE:
		//检测是否需要发送数据
		if (ReSendFlag)
		{
			ReSendFlag = 0;
			Link_AddReq(SOCKET_CLIENT_ID, LINK_REQ_TX, TxBuf, TxLen);
			SocketState = SOCKET_STATE_WAIT_TX;
		}
		else if (CacheBuf.Len)
		{
			PackID++;
			memcpy(TxBuf, &PackID, 4);
			memcpy(TxBuf + 4, gSys.IMEI, 15);
			TxBuf[19] = XorCheck(TxBuf, 19, 0);
			Temp = (LTCC_CacheStruct *)&TxBuf[20];
			ReadRBuffer(&CacheBuf, Temp, 1);
			TxLen = Temp->Len + 20;
			Link_AddReq(SOCKET_CLIENT_ID, LINK_REQ_TX, TxBuf, TxLen);
			SocketState = SOCKET_STATE_WAIT_TX;
		}
		break;
	case SOCKET_STATE_WAIT_TX:
		//等待发送成功

		break;
	case SOCKET_STATE_WAIT_RX:
		//检测服务器应答
		break;
	}
}
