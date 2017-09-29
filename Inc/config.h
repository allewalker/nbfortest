#ifndef __CONFIG_H__
#define __CONFIG_H__
#define __RO__ const
#define TIMER_BASE	(1000)
#define BULK_MAX_LEN (4160)

#define APP_FLASH_START				(0x00004000)
#define BL_PARAM_START				(0x00003000)
#define APP_PARAM_START				(0x00003800)

#define __CUST_BOOTLOADER__			(0)
#define __CUST_MAIN_APP__			(1)
//#define __SYS_START_FROM_BOOTLOADER__
#define __CUST_TYPE__				__CUST_MAIN_APP__

#ifndef __SYS_START_FROM_BOOTLOADER__
#undef __CUST_TYPE__
#define __CUST_TYPE__				__CUST_MAIN_APP__
#endif

#define RX_DMA_UART_ID	UART_ID2

#define DBG_UART_ID UART_ID1
#define AT_UART_ID	UART_ID2
#define AT_BR		(921600)
#define DBG_BR		(921600)

/**********************************ERROR NO**********************************************/
enum
{
	ERROR_AT_CMD_TO = -1,
	ERROR_AT_ANALYZE_STATE = -2,
};

/**********************************IRQ LEVEL*********************************************/
enum
{
	USB_HP_IRQ_LEVEL = 1,
	USB_LP_IRQ_LEVEL,
	SPI_IRQ_LEVEL,
	EXTI_IRQ_LEVEL,

	ADC_DMA_IRQ_LEVEL,
	USART_USER_IRQ_LEVEL,
};
#endif
