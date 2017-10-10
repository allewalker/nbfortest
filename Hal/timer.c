#include "user.h"

void Timer_Config(void)
{
	SysTick_Config(gSys.Var[SYS_FRQ]/TIMER_BASE);
}

void Timer_Task(void *Param)
{
	uint8_t i;
	for (i = 0; i < TIMER_MAX; i++)
	{
		if (gSys.TimerList[i].CallBack && gSys.TimerList[i].ToFlag)
		{
			gSys.TimerList[i].ToFlag = 0;
			gSys.TimerList[i].CallBack((void *)gSys.TimerList[i].Param);
		}
	}
}

int Timer_Start(uint8_t ID, uint32_t To, uint8_t IsRepeat, MyCBFun_t CB)
{
	if (ID >= TIMER_MAX)
	{
		return -1;
	}

	gSys.TimerList[ID].LastTime = To;
	if (IsRepeat)
	{
		gSys.TimerList[ID].ReloadTime = To;
	}
	gSys.TimerList[ID].CallBack = CB;
	gSys.TimerList[ID].Work = 1;
	gSys.TimerList[ID].ToFlag = 0;
	return 0;
}

uint8_t Timer_Check(uint8_t ID)
{
	return gSys.TimerList[ID].ToFlag;
}

void Timer_Switch(uint8_t ID, uint8_t OnOff)
{
	if (ID >= TIMER_MAX)
	{
		return;
	}
	gSys.TimerList[ID].Work = OnOff;
}

void Timer_Restart(uint8_t ID, uint32_t To)
{
	if (ID >= TIMER_MAX)
	{
		return;
	}
	gSys.TimerList[ID].LastTime = To;
	gSys.TimerList[ID].Work = 1;
	gSys.TimerList[ID].ToFlag = 0;
}

void Timer_Del(uint8_t ID)
{
	if (ID >= TIMER_MAX)
	{
		return;
	}
	memset(&gSys.TimerList[ID], 0, sizeof(Timer_CtrlStruct));
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	uint8_t i;
	for(i = 0; i < TIMER_MAX; i++)
	{
		if (gSys.TimerList[i].Work)
		{
			if (gSys.TimerList[i].LastTime)
			{
				gSys.TimerList[i].LastTime--;
			}

			if (!gSys.TimerList[i].LastTime)
			{
				if (gSys.TimerList[i].ReloadTime)
				{
					gSys.TimerList[i].LastTime = gSys.TimerList[i].ReloadTime;
				}
				else
				{
					gSys.TimerList[i].Work = 0;
				}
				gSys.TimerList[i].ToFlag = 1;
			}
		}
	}
}
