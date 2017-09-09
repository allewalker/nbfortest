#include "user.h"
System_Struct gSys;

void bootloader_PrintInfo(void)
{

	int8_t Buf[4][6];
	char *strMon[12] = {"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
	int Day,Year,Month,Hour,Min,Sec,i;
	CmdParam CP;
	memset(&CP, 0, sizeof(CP));
	memset(Buf, 0, sizeof(Buf));
	CP.param_max_len = 6;
	CP.param_max_num = 4;
	CP.param_str = (int8_t *)Buf;
	CmdParseParam(__DATE__, &CP, ' ');
	Month = 0;
	for (i = 0; i < 12; i++)
	{
		if (!strcmp(strMon[i], Buf[0]))
		{
			Month = i + 1;
		}
	}

	if (Buf[1][0])
	{
		Day = strtol(Buf[1], NULL, 10);
		Year = strtol(Buf[2], NULL, 10);
	}
	else
	{
		Day = strtol(Buf[2], NULL, 10);
		Year = strtol(Buf[3], NULL, 10);
	}


	CP.param_num = 0;
	memset(Buf, 0, sizeof(Buf));

	CP.param_str = (int8_t *)Buf;
	CmdParseParam(__TIME__, &CP, ':');
	Hour = strtol(Buf[0], NULL, 10);
	Min = strtol(Buf[1], NULL, 10);
	Sec = strtol(Buf[2], NULL, 10);
	DBG_INFO("\r\nCLK %dMHz Build in %d %d %d %d:%d:%d",
			gSys.Var[SYS_FRQ]/1000000, Year, Month, Day, Hour, Min, Sec);

	if (Year >= 1000)
	{
		Year %= 1000;
	}
	gSys.Var[VERSION] = ( Year * 12 + Month) * 1000000 + Day * 10000 + Hour * 100 + Min;
}

int main(void)
{
	DBG_Config();
	Config_Init();
	DBG("\r\nbootloader build in %s %s", __DATE__, __TIME__);
	NVRAM_Erase(0x08010000);
	while(1)
	{
		//DBG_Send();
	}
}
