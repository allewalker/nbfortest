#include "user.h"
System_Struct gSys;

int main(void)
{
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);//对于freeRTOS必须这样配置
	DBG_Config();
	System_VarInit();
	DBG("\r\nAPP build in %s %s\r\n", __DATE__, __TIME__);
	DBG("SYS_CLK %d ID %x %x %x FlashSize %d\r\n", gSys.Var[SYS_FRQ],
			gSys.Var[CHIP_ID2], gSys.Var[CHIP_ID1], gSys.Var[CHIP_ID0],
			gSys.Var[FLASH_SIZE]);
	DBG_Send();
	while (1)
	{
		Timer_Task();
		DBG_Send();
	}
}
