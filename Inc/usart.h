#ifndef __USART_H__
#define __USART_H__
enum
{
	UART_ID1,
	UART_ID2,
	UART_ID3,
	UART_MAX,
	UART_RX_IRQ = 0,
	UART_TX_DONE_IRQ,
	UART_TX_ERROR_IRQ,
};

typedef struct
{
	MyCBFun_t UartCB;
	uint8_t *TxBuf;
	uint16_t TxLen;
	uint16_t FinishLen;
	uint8_t IsDMAMode;
	uint8_t IsBusy;
}Uart_CtrlStruct;

int Uart_Config(uint8_t UartID, uint32_t BR, MyCBFun_t IrqCB, uint8_t IsDMATx);
void Uart_Tx(uint8_t UartID, void *Src, uint16_t Len);
uint8_t Uart_Rx(uint8_t UartID);
int32_t Uart_RxDMAInit(uint8_t UartID);
int32_t Uart_RxDMAStart(uint8_t UartID, uint8_t *Buf, uint16_t Len);
uint16_t Uart_RxDMAGetSize(uint8_t UartID);
int32_t Uart_RxDMAStop(uint8_t UartID);
#endif
