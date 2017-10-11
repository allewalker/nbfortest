#include "user.h"
char* IP = "115.231.73.229";
#define CONNECT_TO	(120)
#define PORT (9888)
#define UART_TO	(50)
enum
{
	SOCKET_STATE_OFFLINE,
	SOCKET_STATE_CONNECT,
	SOCKET_STATE_ONLINE,
	SOCKET_STATE_WAIT_TX,
	SOCKET_STATE_WAIT_RX,
};

static uint32_t PackID;
static uint8_t TxBuf[1460];
static uint16_t TxLen;
static uint8_t SocketState;
static uint8_t UartOK;
static uint8_t Cache[4096];
static RBuffer CacheBuf;

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
		Link_AddReq(SOCKET_CLIENT_ID, LINK_REQ_CLOSE, IP, PORT);
		SocketState = SOCKET_STATE_OFFLINE;
		break;
	}
}

static void Client_TOCB(void *pData)
{
	UartOK = 1;
	Timer_Switch(CLIENT_TIMER, 0);
}

static void Client_UartCB(void *pData)
{
	uint32_t Type = (uint32_t)pData;
	uint8_t Temp;
	if (Type == UART_RX_IRQ)
	{
		UartOK = 0;
		Temp = Uart_Rx(DTU_UART_ID);
		WriteRBufferForce(&CacheBuf, &Temp, 1);
		Timer_Restart(CLIENT_TIMER, UART_TO);
	}
}

void Client_Init(void)
{
	Link_RegSocket(SOCKET_CLIENT_ID, Client_NotifyCB);
	SocketState = SOCKET_STATE_OFFLINE;
	InitRBuffer(&CacheBuf, Cache, sizeof(Cache), 1);
	Uart_Config(DTU_UART_ID, DTU_BR, Client_UartCB, 1);
	Timer_Start(CLIENT_TIMER, 50, 0, Client_TOCB);
	Timer_Switch(CLIENT_TIMER, 0);
}

void Client_Task(void *pData)
{
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
		if (UartOK && CacheBuf.Len)
		{
			PackID++;
			TxLen = sprintf(TxBuf, "%d,%s,", PackID, gSys.IMEI);
			TxLen += ReadRBuffer(&CacheBuf, &TxBuf[TxLen], 1024);
			Link_AddReq(SOCKET_CLIENT_ID, LINK_REQ_TX, TxBuf, TxLen);
			SocketState = SOCKET_STATE_WAIT_TX;
			TxLen = 0;
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
