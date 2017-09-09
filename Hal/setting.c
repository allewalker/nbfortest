#include "user.h"
#define CPUID_BASE 			(0x1fff7a10)
static unsigned int CRC32_Table[256];

/**
  * @brief  计算buffer的crc校验码
  * @param  Buf 缓冲
  * @param	Size 缓冲区长度
  * @param	CRC32 初始CRC32值
  * @retval 计算后的CRC32
  */
unsigned int CRC32_Calculate(unsigned char *Buf, unsigned int Size, unsigned int CRC32)
{
	return CRC32_Cal(CRC32_Table, Buf, Size, CRC32);
}


/**
  * @brief  系统变量初始化
  * @param  None
  * @retval None
  */
void System_VarInit(void)
{
	RCC_ClocksTypeDef 			RCC_ClocksStatus;
	RCC_GetClocksFreq(&RCC_ClocksStatus);
	gSys.Var[SYS_FRQ] = RCC_ClocksStatus.SYSCLK_Frequency;
	CRC32_CreateTable(CRC32_Table, 0x04C11DB7);


}

void Config_Init(void)
{
	System_VarInit();
}

void Config_Save(void)
{

}
