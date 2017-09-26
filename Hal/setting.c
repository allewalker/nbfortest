#include "user.h"
#define FLASHSIZE_BASE		(0x1FFFF7E0)
#define CPUID_BASE 			(0x1FFFF7E8)

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
	gSys.Var[FLASH_SIZE] = (*(uint16_t *)(FLASHSIZE_BASE)) * 1024;
	gSys.Var[CHIP_ID0] = *(uint32_t *)(CPUID_BASE);
	gSys.Var[CHIP_ID1] = *(uint32_t *)(CPUID_BASE + 4);
	gSys.Var[CHIP_ID2] = *(uint32_t *)(CPUID_BASE + 8);
	CRC32_CreateTable(gSys.CRC32_Table, 0x04C11DB7);
	BL_ParamLS(PARAM_LS_LOAD);
	APP_ParamLS(PARAM_LS_LOAD);
}

void BL_ParamLS(uint8_t IsSave)
{
	BL_ParamStoreStruct BL;
	uint16_t CRC16;
	if (!IsSave)
	{
		memcpy(&BL, (uint8_t *)(FLASH_BASE|BL_PARAM_START), sizeof(BL));
		CRC16 = ~CRC16Cal(BL.Param.Pad, sizeof(BL.Param.Pad), CRC16_START, CRC16_CCITT_GEN, 0);
		if (BL.CRC16 != CRC16)
		{
			DBG_INFO("param error! %x %x recovery", BL.CRC16, CRC16);
		}
		else
		{
			gSys.BLStore = BL.Param.Data;
			return ;
		}
	}
	memset(&BL, 0xff, sizeof(BL));
	BL.Param.Data = gSys.BLStore;
	BL.CRC16 = ~CRC16Cal(BL.Param.Pad, sizeof(BL.Param.Pad), CRC16_START, CRC16_CCITT_GEN, 0);
	NVRAM_Erase(FLASH_BASE|BL_PARAM_START);
	NVRAM_Write(FLASH_BASE|BL_PARAM_START, &BL, sizeof(BL));
	return ;
}

void APP_ParamLS(uint8_t IsSave)
{
	APP_ParamStoreStruct APP;
	uint16_t CRC16;
	if (!IsSave)
	{
		memcpy(&APP, (uint8_t *)(FLASH_BASE|APP_PARAM_START), sizeof(APP));
		CRC16 = ~CRC16Cal(APP.Param.Pad, sizeof(APP.Param.Pad), CRC16_START, CRC16_CCITT_GEN, 0);
		if (APP.CRC16 != CRC16)
		{
			DBG_INFO("Param error! %x %x recovery", APP.CRC16, CRC16);
			memset(&gSys.AppStore, 0, sizeof(gSys.AppStore));
		}
		else
		{
			gSys.AppStore = APP.Param.Data;
			return ;
		}
	}
	memset(&APP, 0xff, sizeof(APP));
	APP.Param.Data = gSys.AppStore;
	APP.CRC16 = ~CRC16Cal(APP.Param.Pad, sizeof(APP.Param.Pad), CRC16_START, CRC16_CCITT_GEN, 0);
	NVRAM_Erase(FLASH_BASE|APP_PARAM_START);
	NVRAM_Write(FLASH_BASE|APP_PARAM_START, &APP, sizeof(APP));
	return ;
}
