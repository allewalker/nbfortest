#include "user.h"
#define RX_DMA_LEN	(1600)
#define AT_RX_LEN	(2048)
#define AT_TX_LEN	(16)
#define AT_UART_ID	UART_ID2
#define AT_BR		921600
typedef struct
{
	uint32_t To;
	uint8_t CmdStr[32];
	uint8_t Param[64];
	uint8_t TxType;
	uint8_t pad[3];
}AT_TxStruct;

typedef struct
{
	uint8_t RxDMABuf[RX_DMA_LEN];
	uint8_t AnalyzeBuf[AT_RX_LEN];
	uint16_t UnAnalyzeSize;			//上次未解析完成的长度
	uint16_t RxSize;				//本次接收的长度
	RBuffer TxQueue;
	AT_TxStruct TxBuf[AT_TX_LEN];

}AT_CtrlStruct;

static AT_CtrlStruct gAT;


static void AT_UartTimerCB(void *pData)
{
	uint16_t RxSize = (RX_DMA_LEN -Uart_RxDMAGetSize(AT_UART_ID));
	if (RxSize != gAT.RxSize)
	{
		gAT.RxSize = RxSize;
	}
	else
	{
		Timer_Switch(AT_RX_TIMER, 0);
		gAT.RxSize++;
		DBG("%d", gAT.RxSize);
		DBG_HexPrintf(gAT.RxDMABuf, gAT.RxSize);
		Uart_RxDMAStop(AT_RX_TIMER);
	}
}

static void AT_UartCB(void *pData)
{
	uint32_t Type = (uint32_t)pData;
	if (Type == UART_RX_IRQ)
	{
		gAT.RxDMABuf[0] = Uart_Rx(UART_ID2);
		gAT.RxSize = 0;
		Uart_RxDMAStart(AT_UART_ID, gAT.RxDMABuf + 1, RX_DMA_LEN - 1);
		Timer_Switch(AT_RX_TIMER, 1);
	}
}


void AT_Init(void)
{
	InitRBuffer(&gAT.TxQueue, gAT.TxBuf, AT_TX_LEN, sizeof(AT_TxStruct));
	Uart_Config(AT_UART_ID, AT_BR, AT_UartCB, 1);
	Uart_RxDMAInit(AT_UART_ID);
	Timer_Start(AT_RX_TIMER, 5, 1, AT_UartTimerCB);
	Timer_Switch(AT_RX_TIMER, 0);
}
