#include "user.h"

#define AT_READY			"RDY"
#define AT_SOCKET_REC		"+RECEIVE"
#define AT_IP_STATE			"STATE:"
#define AT_SOCKET_STATE		"C:"
#define AT_PDP_DEACT		"+PDP: DEACT"

const char *Link_InitCmdQueue[] =
{
		"E0",
		"CMEE=1",
		"CFUN=1",
		"CIPMUX=1",
};

static Link_CtrlStruct gLink;

static void Link_ATCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Result)
	{
		DBG_INFO("%d", Result->Result);
	}
}

static void Link_TOCB(void *pData)
{
	gLink.ToFlag = 1;
}

static void Link_SubActive(void)
{
	switch (gLink.SubState)
	{
	case LINK_SUB_STATE_WAIT_PIN:
		break;
	case LINK_SUB_STATE_WAIT_CGATT:
		break;
	case LINK_SUB_STATE_WAIT_IP_START:
		break;
	case LINK_SUB_STATE_WAIT_IP_GPRSACT:
		break;
	case LINK_SUB_STATE_WAIT_IP_STATUS:
		break;
	}
}


static void Link_SubComm(void)
{

}

static void Link_SubSleep(void)
{

}

static void Link_SubInit(void)
{
	switch (gLink.SubState)
	{
	case LINK_SUB_STATE_POWER_DOWN:
		if (gLink.ToFlag)
		{
			gLink.ToFlag = 0;
			gLink.SubState = LINK_SUB_STATE_POWER_ON;
			IO_Write(PIN_IOT_POWER, 1);
			Timer_Start(LINK_TIMER, 10000, 0, Link_TOCB);
		}
		break;
	case LINK_SUB_STATE_POWER_ON:
#if 0
		if (IO_Read(PIN_IOT_READY))
		{
			gLink.Stage = LINK_STATE_WAIT_READY;
			Timer_Restart(LINK_TIMER, 0);
		}
#else
		gLink.SubState = LINK_SUB_STATE_WAIT_READY;
#endif
		DBG_INFO("wait for ready");
		break;
	case LINK_SUB_STATE_WAIT_READY:
		if (gLink.ToFlag)
		{
			DBGF;
			gLink.ToFlag = 0;
			Link_Restart();
		}
		break;
	case LINK_SUB_STATE_INIT_QUEUE:
		AT_AddDirCmd((int8_t *)Link_InitCmdQueue[0], Link_ATCB);
		gLink.MainState = LINK_MAIN_STATE_ACTIVE;
		break;
	default:
		DBGF;
		Link_Restart();
		break;
	}
}

void Link_Task(void *Param)
{
	switch (gLink.MainState)
	{
	case LINK_MAIN_STATE_COMM:
		Link_SubComm();
		break;
	case LINK_MAIN_STATE_ACTIVE:
		Link_SubActive();
		break;
	case LINK_MAIN_STATE_INIT:
		Link_SubInit();
		break;
	case LINK_MAIN_STATE_SLEEP:
		Link_SubSleep();
		break;
	default:
		break;

	}
}

void Link_Restart(void)
{
	gLink.MainState = LINK_MAIN_STATE_INIT;
	gLink.SubState = LINK_SUB_STATE_POWER_DOWN;
	IO_Write(PIN_IOT_POWER, 0);
	Timer_Start(LINK_TIMER, 100, 0, Link_TOCB);
}

void Link_Init(void)
{
	gSys.LinkCtrl = &gLink;
	InitRBuffer(&gLink.ReqList, gLink.ReqBuf, SOCKET_MAX, sizeof(Link_ReqStruct));
	Link_Restart();
}


void Link_RxData(uint8_t *Data, uint16_t Len)
{
	Link_NotifyStruct Notify;
	if (gLink.RxSocketID < SOCKET_MAX)
	{
		if (gLink.NotifyCB[Notify.SocketID])
		{
			Notify.SocketID = gLink.RxSocketID;
			Notify.RxData = Data;
			Notify.RxDataLen = Len;
			Notify.Event = LINK_EVENT_RX;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
		}
	}
}

void Link_TxData(void)
{
	if ( (gLink.TxSocketID >= 0) && (gLink.TxSocketID < SOCKET_MAX) )
	{
		if (gLink.TxBuf && gLink.TxLen)
		{
			AT_TxRawData(gLink.TxBuf, gLink.TxLen);
		}
	}
}

void Link_UrcAnalyze(int8_t *Str)
{
	uint8_t Temp = Str[0] - '0';
	Link_NotifyStruct Notify;
	if (Temp < SOCKET_MAX)
	{
		Notify.SocketID = Temp;
		if (!gLink.NotifyCB[Notify.SocketID])
		{
			return ;
		}
		if ( strstr(Str + 2, "CONNECT OK") || strstr(Str + 2, "ALREADY CONNECT") )
		{
			Notify.Event = LINK_EVENT_CONNECT_OK;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			return ;
		}
		else if ( strstr(Str + 2, "CONNECT FAIL") )
		{
			Notify.Event = LINK_EVENT_CONNECT_FAIL;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			return ;
		}
		else if ( strstr(Str + 2, "SEND OK") )
		{
			gLink.TxSocketID = 0xff;
			gLink.TxBuf = NULL;
			Notify.Event = LINK_EVENT_TX_OK;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			AT_FinishCmd();
			return ;
		}
		else if ( strstr(Str + 2, "SEND FAIL") )
		{
			gLink.TxSocketID = 0xff;
			gLink.TxBuf = NULL;
			Notify.Event = LINK_EVENT_TX_FAIL;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			AT_FinishCmd();
			return ;
		}
		else if ( strstr(Str + 2, "CLOSE OK") || strstr(Str + 2, "CLOSE FAIL") )
		{
			Notify.Event = LINK_EVENT_CLOSE;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			AT_FinishCmd();
			return ;
		}
		else if ( strstr(Str + 2, "CLOSED") )
		{
			Notify.Event = LINK_EVENT_CLOSE;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			return ;
		}
	}
	else if (!strcmp(Str, AT_SOCKET_REC))
	{
		gLink.RxSocketID = 0xffffffff;
		sscanf(Str, "+RECEIVE,%d,%d:",&gLink.RxSocketID, &gLink.RxLen);
		if (gLink.RxSocketID != 0xffffffff)
		{
			AT_RxRawData(gLink.RxLen);
		}
	}
	else if (!strcmp(Str, AT_READY))
	{
		if (gLink.SubState == LINK_SUB_STATE_WAIT_READY)
		{
			DBG_INFO("link ready start init queue");
			Timer_Switch(LINK_TIMER, 0);
			gLink.SubState = LINK_SUB_STATE_INIT_QUEUE;
		}
	}
	else if (!strcmp(Str, AT_PDP_DEACT))
	{
		gLink.IPState = LINK_IP_PDP_DEACT;
	}
	else if (!strcmp(Str, AT_IP_STATE))
	{
		if (strstr(Str, "INITIAL"))
		{
			gLink.IPState = LINK_IP_INITIAL;
			return;
		}
		if (strstr(Str, "START"))
		{
			gLink.IPState = LINK_IP_START;
			return;
		}
		if (strstr(Str, "CONFIG"))
		{
			gLink.IPState = LINK_IP_CONFIG;
			return;
		}
		if (strstr(Str, "GPRSACT"))
		{
			gLink.IPState = LINK_IP_GPRSACT;
			return;
		}
		if (strstr(Str, "STATUS"))
		{
			gLink.IPState = LINK_IP_STATUS;
			return;
		}
		if (strstr(Str, "PROCESSING"))
		{
			gLink.IPState = LINK_IP_PROCESSING;
			return;
		}
		if (strstr(Str, "PDP DEACT"))
		{
			gLink.IPState = LINK_IP_PDP_DEACT;
			return;
		}
	}
	else if (!strcmp(Str, AT_SOCKET_STATE))
	{
	}
}
