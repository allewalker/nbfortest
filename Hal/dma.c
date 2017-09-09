#include "user.h"


void DMA1_Channel2_IRQHandler(void)
{
	uint32_t SR = DMA1->ISR;
	DMA1->IFCR = DMA_IFCR_CGIF2|DMA_IFCR_CTCIF2|DMA_IFCR_CHTIF2|DMA_IFCR_CTEIF2;
	if (gSys.Uart[UART_ID2].IsDMAMode)
	{
		gSys.Uart[UART_ID2].IsBusy = 0;
		if (SR & DMA_ISR_TCIF2)
		{
			SR = UART_TX_DONE_IRQ;
			gSys.Uart[UART_ID2].UartCB((void *)SR);
		}
		else
		{
			SR = UART_TX_ERROR_IRQ;
			gSys.Uart[UART_ID2].UartCB((void *)SR);
		}
	}
}

void DMA1_Channel4_IRQHandler(void)
{
	uint32_t SR = DMA1->ISR;
	DMA1->IFCR = DMA_IFCR_CGIF4|DMA_IFCR_CTCIF4|DMA_IFCR_CHTIF4|DMA_IFCR_CTEIF4;
	if (gSys.Uart[UART_ID1].IsDMAMode)
	{
		gSys.Uart[UART_ID1].IsBusy = 0;
		if (SR & DMA_ISR_TCIF4)
		{
			SR = UART_TX_DONE_IRQ;
			gSys.Uart[UART_ID1].UartCB((void *)SR);
		}
		else
		{
			SR = UART_TX_ERROR_IRQ;
			gSys.Uart[UART_ID1].UartCB((void *)SR);
		}
	}
}

void DMA1_Channel7_IRQHandler(void)
{
	uint32_t SR = DMA1->ISR;
	DMA1->IFCR = DMA_IFCR_CGIF7|DMA_IFCR_CTCIF7|DMA_IFCR_CHTIF7|DMA_IFCR_CTEIF7;
	if (gSys.Uart[UART_ID3].IsDMAMode)
	{
		gSys.Uart[UART_ID3].IsBusy = 0;
		if (SR & DMA_ISR_TCIF7)
		{
			SR = UART_TX_DONE_IRQ;
			gSys.Uart[UART_ID3].UartCB((void *)SR);
		}
		else
		{
			SR = UART_TX_ERROR_IRQ;
			gSys.Uart[UART_ID3].UartCB((void *)SR);
		}
	}
}

