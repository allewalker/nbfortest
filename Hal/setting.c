#include "user.h"
#define CPUID_BASE 			(0x1fff7a10)
static unsigned int CRC32_Table[256];

/**
  * @brief  ����buffer��crcУ����
  * @param  Buf ����
  * @param	Size ����������
  * @param	CRC32 ��ʼCRC32ֵ
  * @retval ������CRC32
  */
unsigned int CRC32_Calculate(unsigned char *Buf, unsigned int Size, unsigned int CRC32)
{
	return CRC32_Cal(CRC32_Table, Buf, Size, CRC32);
}


/**
  * @brief  ϵͳ������ʼ��
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
