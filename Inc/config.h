#ifndef __CONFIG_H__
#define __CONFIG_H__
#define __RO__ const
#define TIMER_BASE	(1000)
#define FLASH_PAGE_SIZE	(1024)
#define FLASH_ATOM_SIZE	(128)
#define USERDATA_START ((u32)0x0800DC00) //55K
#define BULK_MAX_LEN	(4160)	//自定义的块传输包最大长度 4k+64
#define DBG_USART_BR	(921600)
#define USER_USART_BR	(460800)
#define WIFI_USART_BR	(74880)
#define WIFIUSART_TXBUF_LEN	(2048)
#define WIFIUSART_RXBUF_LEN	(64)

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
