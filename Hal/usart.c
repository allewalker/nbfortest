#include "user.h"

static const USART_TypeDef* hwUart[3] = {USART1, USART2, USART3};
static const DMA_Channel_TypeDef* hwUartTxDMA[3] = {DMA1_Channel4, DMA1_Channel7, DMA1_Channel2};
static const DMA_Channel_TypeDef* hwUartRxDMA[3] = {DMA1_Channel5, DMA1_Channel6, DMA1_Channel3};
void Uart_DummyCB(void *pData)
{

}

int Uart_Config(uint8_t UartID, uint32_t BR, MyCBFun_t IrqCB, uint8_t IsDMATx)
{
	USART_InitTypeDef USART_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	uint8_t IrqChannel, DMAIrqChannel;
	uint32_t ClearFlag;
	USART_TypeDef* Uart;
	DMA_Channel_TypeDef* DMA;
	if (UartID >= UART_MAX)
	{
		return -1;
	}
	Uart = (USART_TypeDef* )hwUart[UartID];
	DMA = (DMA_Channel_TypeDef* )hwUartTxDMA[UartID];
	USART_StructInit(&USART_InitStructure);
	USART_InitStructure.USART_BaudRate = BR;
	USART_Init( Uart, &USART_InitStructure );

	switch (UartID)
	{
	case UART_ID1:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init( GPIOA, &GPIO_InitStructure );
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init( GPIOA, &GPIO_InitStructure );
		IrqChannel = USART1_IRQn;
		DMAIrqChannel = DMA1_Channel4_IRQn;
		ClearFlag = DMA_IFCR_CGIF4|DMA_IFCR_CTCIF4|DMA_IFCR_CHTIF4|DMA_IFCR_CTEIF4;
		break;
	case UART_ID2:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOA, &GPIO_InitStructure);
		IrqChannel = USART2_IRQn;
		DMAIrqChannel = DMA1_Channel7_IRQn;
		ClearFlag = DMA_IFCR_CGIF7|DMA_IFCR_CTCIF7|DMA_IFCR_CHTIF7|DMA_IFCR_CTEIF7;
		break;
	case UART_ID3:
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		IrqChannel = USART3_IRQn;
		DMAIrqChannel = DMA1_Channel2_IRQn;
		ClearFlag = DMA_IFCR_CGIF2|DMA_IFCR_CTCIF2|DMA_IFCR_CHTIF2|DMA_IFCR_CTEIF2;
		break;
	}

	USART_ITConfig(Uart, USART_IT_RXNE, ENABLE);
//	USART_ITConfig(WIFIUSART, USART_IT_TC, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = IrqChannel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = USART_USER_IRQ_LEVEL;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	if (IsDMATx)
	{
		DMA_Cmd(DMA, DISABLE);
		DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&Uart->DR;//外设地址
		DMA_InitStructure.DMA_MemoryBaseAddr = NULL;//内存地址
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;//dma传输方向单向
		DMA_InitStructure.DMA_BufferSize = 0;//设置DMA在传输时缓冲区的长度 word
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//设置DMA的外设递增模式，一个外设
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置DMA的内存递增模式，
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据字长
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据字长
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//设置DMA的传输模式
		DMA_InitStructure.DMA_Priority = DMA_Priority_High;//设置DMA的优先级别
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//设置DMA的2个memory中的变量互相访问
		DMA_Init(DMA, &DMA_InitStructure);
		USART_DMACmd(Uart, USART_DMAReq_Tx, ENABLE);
		gSys.Uart[UartID].IsDMAMode = 1;
		NVIC_InitStructure.NVIC_IRQChannel = DMAIrqChannel;
		NVIC_Init(&NVIC_InitStructure);
		DMA_ITConfig(DMA, DMA_IT_TC, ENABLE);
		DMA_ITConfig(DMA, DMA_IT_TE, ENABLE);
		DMA1->IFCR = ClearFlag;
	}
	else
	{
		gSys.Uart[UartID].IsDMAMode = 0;
	}

	if (IrqCB)
	{
		gSys.Uart[UartID].UartCB = IrqCB;
	}
	else
	{
		gSys.Uart[UartID].UartCB = Uart_DummyCB;
	}
	gSys.Uart[UartID].IsBusy = 0;
	USART_Cmd(Uart, ENABLE);
	return 0;
}

uint8_t Uart_Rx(uint8_t UartID)
{
	USART_TypeDef* Uart = (USART_TypeDef* )hwUart[UartID];
	return Uart->DR;
}

int32_t Uart_RxDMAInit(uint8_t UartID)
{
	DMA_Channel_TypeDef *DMA;
	USART_TypeDef* Uart;
	DMA_InitTypeDef DMA_InitStructure;
#ifdef __UART2_RX_DMA__
	if (UartID != UART_ID2)
	{
		return -1;
	}
	Uart = (USART_TypeDef* )hwUart[UartID];
	DMA = (DMA_Channel_TypeDef *)hwUartRxDMA[UartID];
	DMA_Cmd(DMA, DISABLE);
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&Uart->DR;//外设地址
	DMA_InitStructure.DMA_MemoryBaseAddr = NULL;//内存地址
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;//dma传输方向单向
	DMA_InitStructure.DMA_BufferSize = 0;//设置DMA在传输时缓冲区的长度 word
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//设置DMA的外设递增模式，一个外设
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;//设置DMA的内存递增模式，
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//外设数据字长
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//内存数据字长
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;//设置DMA的传输模式
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;//设置DMA的优先级别
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;//设置DMA的2个memory中的变量互相访问
	DMA_Init(DMA, &DMA_InitStructure);
	USART_DMACmd(Uart, USART_DMAReq_Rx, ENABLE);
	DMA1->IFCR = 0XFFFFFFFF;
	return 0;
#endif
	return -1;
}

int32_t Uart_RxDMAStart(uint8_t UartID, uint8_t *Buf, uint16_t Len)
{
	DMA_Channel_TypeDef *DMA;
	USART_TypeDef* Uart;
#ifdef __UART2_RX_DMA__
	if (UartID != UART_ID2)
	{
		return -1;
	}
	Uart = (USART_TypeDef* )hwUart[UartID];
	DMA = (DMA_Channel_TypeDef *)hwUartRxDMA[UartID];
	USART_ITConfig(Uart, USART_IT_RXNE, DISABLE);
	DMA->CMAR = (uint32_t)Buf;
	DMA->CNDTR = Len;
	DMA_Cmd(DMA, ENABLE);
	return 0;
#endif
	return -1;
}

uint16_t Uart_RxDMAGetSize(uint8_t UartID)
{
	DMA_Channel_TypeDef *DMA;
#ifdef __UART2_RX_DMA__
	if (UartID != UART_ID2)
	{
		return 0;
	}
	DMA = (DMA_Channel_TypeDef *)hwUartRxDMA[UartID];
	return DMA->CNDTR;
#endif
	return 0;
}

int32_t Uart_RxDMAStop(uint8_t UartID)
{
	DMA_Channel_TypeDef *DMA;
	USART_TypeDef* Uart;
#ifdef __UART2_RX_DMA__
	if (UartID != UART_ID2)
	{
		return -1;
	}
	Uart = (USART_TypeDef* )hwUart[UartID];
	DMA = (DMA_Channel_TypeDef *)hwUartRxDMA[UartID];
	DMA_Cmd(DMA, DISABLE);
	USART_ITConfig(Uart, USART_IT_RXNE, ENABLE);
	return 0;
#endif
	return -1;
}

void Uart_Tx(uint8_t UartID, void *Src, uint16_t Len)
{
	DMA_Channel_TypeDef *DMA;
	USART_TypeDef* Uart = (USART_TypeDef* )hwUart[UartID];
	if (UartID >= UART_MAX)
	{
		return ;
	}
	gSys.Uart[UartID].IsBusy = 1;
	if (gSys.Uart[UartID].IsDMAMode)
	{
		DMA = (DMA_Channel_TypeDef *)hwUartTxDMA[UartID];
		DMA_Cmd(DMA, DISABLE);
		DMA->CMAR = (uint32_t)Src;
		DMA->CNDTR = Len;
		DMA_Cmd(DMA, ENABLE);
	}
	else
	{
		gSys.Uart[UartID].TxBuf = (uint8_t *)Src;
		gSys.Uart[UartID].TxLen = Len;
		gSys.Uart[UartID].FinishLen = 0;

		Uart->DR = gSys.Uart[UartID].TxBuf[0];
		USART_ITConfig(Uart, USART_IT_TXE, ENABLE);
		USART_ITConfig(Uart, USART_IT_TC, ENABLE);
	}
}

void Uart_IrqHandler(uint8_t UartID)
{
	USART_TypeDef* Uart = (USART_TypeDef* )hwUart[UartID];
	uint16_t SR = Uart->SR;
	uint32_t IRQ;

	if (SR & USART_SR_TC)
	{
		IRQ = UART_TX_DONE_IRQ;
		Uart->SR &= ~USART_SR_TC;
		USART_ITConfig(Uart, USART_IT_TC, DISABLE);
		gSys.Uart[UartID].IsBusy = 0;
		gSys.Uart[UartID].UartCB((void *)IRQ);
	}

	if (SR & USART_SR_RXNE)
	{
		IRQ = UART_RX_IRQ;
		if (gSys.Uart[UartID].UartCB == Uart_DummyCB)
		{
			IRQ = Uart_Rx(UartID);
		}
		else
		{
			gSys.Uart[UartID].UartCB((void *)IRQ);
		}
	}
	else if (SR & USART_SR_TXE)
	{
		gSys.Uart[UartID].FinishLen++;
		if (gSys.Uart[UartID].FinishLen >= gSys.Uart[UartID].TxLen)
		{
			USART_ITConfig(Uart, USART_IT_TXE, DISABLE);
		}
		else
		{
			Uart->DR = gSys.Uart[UartID].TxBuf[gSys.Uart[UartID].FinishLen];
		}
	}

}

//串口1
void USART1_IRQHandler( void )
{
	Uart_IrqHandler(UART_ID1);
	return;
}

//串口1
void USART2_IRQHandler( void )
{
	Uart_IrqHandler(UART_ID2);
	return;
}

//串口1
void USART3_IRQHandler( void )
{
	Uart_IrqHandler(UART_ID3);
	return;
}
