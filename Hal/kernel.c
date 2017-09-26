#include "user.h"
System_Struct gSys;



__asm void MSR_MSP ( uint32_t ulAddr )
{
    MSR MSP, r0 			                   //set Main Stack value
    BX r14
}

//跳转到应用程序段
//ulAddr_App:用户代码起始地址.
void IAP_ExecuteApp ( uint32_t ulAddr_App )
{
	MainFun_t MainFun;

	if ( ( ( * ( vu32 * ) ulAddr_App ) & 0x2FFE0000 ) == 0x20000000 )	  //检查栈顶地址是否合法.
	{
		MainFun = ( MainFun_t ) * ( vu32 * ) ( ulAddr_App + 4 );	//用户代码区第二个字为程序开始地址(复位地址)
		MSR_MSP ( * ( vu32 * ) ulAddr_App );					                            //初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址)
		MainFun ();								                                    	//跳转到APP.
	}
}

void Upgrade_MainFlash(void)
{
	uint32_t CRC32;
	uint32_t AppAddr;
	uint32_t UpgradeAddr;
	uint32_t SectorLen;
	uint32_t i;
	uint8_t Retry;

	if (gSys.Var[FLASH_SIZE] > (128 * 1024))
	{
		SectorLen = 2048;
	}
	else
	{
		SectorLen = 1024;
	}

	if (gSys.AppStore.UpgradeEntry % 4)
	{
		DBG("upgrade entry addr error! %x\r\n", gSys.AppStore.UpgradeEntry);
		goto CLEAR_UPGRADE;
	}

	if ( (gSys.AppStore.UpgradeEntry + gSys.AppStore.UpgradeSector * SectorLen) > gSys.Var[FLASH_SIZE])
	{
		DBG("upgrade file len error! %d %d\r\n",
				gSys.AppStore.UpgradeEntry + gSys.AppStore.UpgradeSector * SectorLen,
				gSys.Var[FLASH_SIZE]);
		goto CLEAR_UPGRADE;
	}

	if (gSys.AppStore.UpgradeEntry < ((gSys.Var[FLASH_SIZE] + APP_FLASH_START) / 2))
	{
		DBG("upgrade entry error! %x %x\r\n", gSys.AppStore.UpgradeEntry,
				(gSys.Var[FLASH_SIZE] + APP_FLASH_START) / 2);
		goto CLEAR_UPGRADE;
	}

	CRC32 = ~CRC32_Cal(gSys.CRC32_Table, (uint8_t *)gSys.AppStore.UpgradeEntry,
			gSys.AppStore.UpgradeSector * SectorLen,
			CRC32_START);
	if (CRC32 != gSys.AppStore.UpgradeCRC32)
	{
		DBG("upgrade crc32 error! %x %x\r\n", gSys.AppStore.UpgradeCRC32,
				CRC32);
		goto CLEAR_UPGRADE;
	}

	for (i = 0; i < gSys.AppStore.UpgradeSector; i++)
	{
		AppAddr = FLASH_BASE|APP_FLASH_START + SectorLen * i;
		UpgradeAddr = gSys.AppStore.UpgradeEntry + SectorLen * i;
		for (Retry = 0; Retry < 3; Retry++)
		{
			NVRAM_Erase(AppAddr);
			NVRAM_Write(AppAddr, (void *)UpgradeAddr, SectorLen);
			if (memcmp((void *)AppAddr, (void *)UpgradeAddr, SectorLen))
			{
				DBG("sector %d copy error!", i);
			}
			else
			{
				break;
			}
		}
	}
	DBG("upgrade done!");
CLEAR_UPGRADE:
	gSys.AppStore.UpgradeReq = 0;
	gSys.AppStore.UpgradeType = 0;
	gSys.AppStore.UpgradeEntry = 0;
	gSys.AppStore.UpgradeSector = 0;
	gSys.AppStore.UpgradeCRC32 = 0;
	APP_ParamLS(PARAM_LS_SAVE);
}

int main(void)
{
	DBG_Config();
	System_VarInit();
	DBG("\r\nbootloader build in %s %s\r\n", __DATE__, __TIME__);
	DBG("SYS_CLK %d ID %x %x %x FlashSize %d\r\n", gSys.Var[SYS_FRQ],
			gSys.Var[CHIP_ID2], gSys.Var[CHIP_ID1], gSys.Var[CHIP_ID0],
			gSys.Var[FLASH_SIZE]);

	DBG_Send();
	while (1)
	{

	}
}
