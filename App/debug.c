/*
 * 通用调试模块，只负责串口输出，使用循环缓冲和中断方式输出
 * 针对不同的平台，需要修改硬件初始化方式和中断服务函数
 */

#include "user.h"
#define FIND_HEAD 0
#define FIND_TAIL 64
#define DBG_UART_ID UART_ID1

///* Private typedef -----------------------------------------------------------*/
//typedef char * uart_va_list;
///* Private define ------------------------------------------------------------*/
//#define uart_sizeof_int(n)   		((sizeof(n) + sizeof(int) - 1) & ~(sizeof(int) - 1))
//#define uart_va_start(ap,v)  		(ap = (uart_va_list)&v +uart_sizeof_int(v))
//#define uart_va_arg(ap,t)    		(*(t *)((ap += uart_sizeof_int(t)) - uart_sizeof_int(t)))
//#define uart_va_end(ap)      		(ap = (uart_va_list)0)


void DBG_CB(void *pData)
{
	uint32_t Type = (uint32_t)pData;
	uint8_t RxData;
	if (Type == UART_RX_IRQ)
	{
		RxData = Uart_Rx(DBG_UART_ID);
	}
}
/**
  * @brief  调试初始化
  * @param  None
  * @retval None
  */
void DBG_Config(void)
{
	InitRBuffer(&gSys.TraceBuffer, gSys.TraceData, DBG_BUF_SIZE, 1);
	Uart_Config(DBG_UART_ID, DBG_BR, DBG_CB, 1);

}


/**
  * @brief  调试信息输出
  * @param  None
  * @retval None
  */
int32_t DBG_Send(void)
{
	uint16_t TxLen;
	if (!gSys.Uart[DBG_UART_ID].IsBusy && gSys.TraceBuffer.Len)
	{
		TxLen = ReadRBuffer(&gSys.TraceBuffer, gSys.TraceDMA, gSys.TraceBuffer.Len);
		Uart_Tx(DBG_UART_ID, gSys.TraceDMA, TxLen);
		return TxLen;
	}
	else
	{
		return -1;
	}
}


void DBG_HexPrintf(unsigned char *data, unsigned int len)
{
	int i;
	char hex[3];
	for (i = 0; i < len; i++) {
		hex[0] =(data[i] & 0xf0) >> 4;
		if (hex[0]<10) {
			hex[0] += '0';
		} else {
			hex[0] += 'a' - 10;
		}
		hex[1] =data[i] & 0x0f;
		if (hex[1]<10) {
			hex[1] += '0';
		} else {
			hex[1] += 'a' - 10;
		}
		hex[2] = ' ';
		WriteRBufferForce(&gSys.TraceBuffer,(uint8_t *)hex, 3);


	}
	WriteRBufferForce(&gSys.TraceBuffer,"\r\n", 2);
//	DBG_Send();

}

void DBG_Printf(const char* format, ...)
{
#if 1
	char c;
	char *s = 0;
	int d = 0, i = 0, p = 0;
	unsigned int ud = 0;
	unsigned char ch[sizeof(int) * 2 + 2] = {0};
	unsigned char in[16] = {0};
	unsigned char int_max;
	unsigned char flag;
	va_list ap;

	va_start(ap, format);

	while (* format)
	{
		if (* format != '%')
		{
			WriteRBufferForce(&gSys.TraceBuffer, (uint8_t *)format, 1);

			format += 1;
			continue;
		}
		switch (*(++format))
		{
			case 's':
			case 'S':
				s = va_arg(ap, char *);
				for ( ; *s; s++)
				{
					WriteRBufferForce(&gSys.TraceBuffer,(uint8_t *) s, 1);

				}
				break;
			case 'c':
			case 'C':
				c = va_arg(ap, char);
				WriteRBufferForce(&gSys.TraceBuffer, (uint8_t *)&c, 1);

				break;
			case 'x':
			case 'X':
				ud = va_arg(ap, int);
				if (ud > 0xffff)
				{
					p = 8;
				}
				else if (ud > 0xff)
				{
					p = 4;
				}
				else
				{
					p = 2;
				}
				for(i = 0; i < p; i++)
				{
					ch[i] = (unsigned char)(ud&0x0f) + '0';
					if(ch[i] > '9')
						ch[i] += 7;
					ud >>= 4;
				}

				for(i = p; i > 0; i--)
				{
					WriteRBufferForce(&gSys.TraceBuffer, &ch[i -1], 1);

				}
				break;
			case 'd':
			case 'D':
				d = va_arg(ap, int);
				if (d < 0) {
					flag = 1;
				} else {
					flag = 0;
				}
				d = abs(d);
				int_max = 0;
				for(i = 0; i < 16; i++)
				{

					in[i] = d%10 + '0';
					d = d/10;
					int_max++;
					if (!d)
						break;
				}
				if (flag)
				{
					flag = '-';
					WriteRBufferForce(&gSys.TraceBuffer, &flag, 1);

				}
				for(i = int_max; i > 0; i--)
				{
					WriteRBufferForce(&gSys.TraceBuffer, &in[i -1], 1);

				}
				break;
			default:
				WriteRBufferForce(&gSys.TraceBuffer, (uint8_t *)format, 1);

				break;
		}
		format++;
	}
	va_end(ap);
#else
	int8_t buf[128];
	uint8_t Len;
	va_list ap;
	va_start(ap, format);
	Len = vsnprintf(buf, sizeof(buf), format, ap);
	va_end(ap);
	WriteRBufferForce(&gSys.TraceBuffer, buf, Len);
#endif
//	DBG_Send();

}

/**
  * @brief  获取本地调试信息
  * @param  Data 调试信息
  * @param  Len 调试信息长度
  * @retval None
  */
int DBG_GetData(void *Data, unsigned int Len)
{
	return ReadRBuffer(&gSys.TraceBuffer, Data, Len);
}

/**
  * @brief  获取本地调试信息长度
  * @param  None
  * @retval None
  */
int DBG_ReturnBufLen(void)
{
	return gSys.TraceBuffer.Len;
}


