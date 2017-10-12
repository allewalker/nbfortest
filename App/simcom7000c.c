#include "user.h"
#ifdef __NB_SIMCOM7000C__

#define AT_READY			"RDY"
#define AT_SOCKET_REC		"+RECEIVE"
#define AT_IP_STATE			"STATE:"
#define AT_SOCKET_STATE		"C:"
#define AT_PDP_DEACT		"+PDP: DEACT"
#define AT_RESET_TO			120
#define AT_TCP_TO			80
#define AT_CGATT_PERIOD		3
enum
{
	LINK_CHECK_START,
	LINK_CHECK_RUN,
	LINK_CHECK_DONE,
	LINK_CHECK_ERROR,
	LINK_CHECK_WAIT,
};

const char *Link_InitCmdQueue[] =
{
		"ATE0\r",
		"CMEE=1",
		"CFUN=1",
		"CNMP=38",
		"CMNB=2",
};

const char *Link_SocketCmdQueue[] =
{
		//"CIICRMODE=2",
		"CGNAPN",
		"CIPMUX=1",
		"CIPHEAD=1",
		"CIPQSEND=0",
		"CSTT=\"\"",
		"CIICR"
};

static Link_CtrlStruct gLink;

static void Link_TimerStart(uint32_t To);
static void Link_TimerStop();

static void Link_ATDummyCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Result)
	{
		DBG_INFO("%d", Result->Result);
	}
}

static void Link_InitFinishCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	Link_TimerStop();
	if (Result->Result)
	{
		DBG_INFO("%d", Result->Result);
	}
	else
	{
		if (gLink.SubState == LINK_SUB_STATE_WAIT_INIT_DONE)
		{
			DBG_INFO("link init queue done");
			gLink.SubState = LINK_SUB_STATE_WAIT_PIN;
		}
	}
}

static void Link_GetIMSICB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Data && Result->Size)
	{
		if (IsDigit(Result->Data[0]))
		{
			Result->Data[Result->Size] = 0;
			strcpy(gSys.IMSI, Result->Data);
			DBG_INFO("%s", gSys.IMSI);
		}
	}
	else
	{
		if (Result->Result)
		{
			DBG_INFO("%d", Result->Result);
		}
	}
}


static void Link_GetIMEICB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Data && Result->Size)
	{
		if (IsDigit(Result->Data[0]))
		{
			Result->Data[Result->Size] = 0;
			strcpy(gSys.IMEI, Result->Data);
			DBG_INFO("%s", gSys.IMEI);
		}
	}
	else
	{
		if (Result->Result)
		{
			DBG_INFO("%d", Result->Result);
		}
	}
}

static void Link_PinCheckCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	Link_TimerStop();
	if (Result->Data && Result->Size)
	{
		if (strstr(Result->Data, "READY"))
		{
			DBG_INFO("pin ok");
			gLink.PinCheck = LINK_CHECK_DONE;
		}
		else
		{
			gLink.PinCheck = LINK_CHECK_ERROR;
		}
	}
	else
	{
		if (Result->Result)
		{
			DBG_INFO("%d", Result->Result);
			gLink.PinCheck = LINK_CHECK_ERROR;
		}
	}
}

static void Link_AttachCheckCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	Link_TimerStop();
	if (Result->Data && Result->Size)
	{
		//DBG_INFO("%s", Result->Data);
		if (strstr(Result->Data, "+CGATT: 1"))
		{
			DBG_INFO("attach ok");
			gLink.AttachCheck = LINK_CHECK_DONE;
		}
		else
		{
			gLink.AttachCheck = LINK_CHECK_ERROR;
		}
	}
	else
	{
		if (Result->Result)
		{
			DBG_INFO("%d", Result->Result);
			gLink.AttachCheck = LINK_CHECK_ERROR;
		}
	}
}

static void Link_ActiveCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Data && Result->Size)
	{
		Result->Data[Result->Size] = 0;
		DBG_INFO("%s", Result->Data);
		AT_FinishCmd();
		gLink.ActiveCheck = 1;
	}
	else
	{
		if (Result->Result)
		{
			DBG_INFO("%d", Result->Result);
			Link_Restart();
		}
	}
}

static void Link_TOCB(void *pData)
{
	gLink.ToFlag = 1;
	Link_TimerStop();
}

static void Link_ResetTOCB(void *pData)
{
	DBGF;
	Link_Restart();
}

static void Link_TimerStart(uint32_t To)
{
	gLink.ToFlag = 0;
	Timer_Restart(LINK_TIMER, To);
}
static void Link_TimerStop()
{
	Timer_Switch(LINK_TIMER, 0);
}

static void Link_SubActive(void)
{
	int i;
	switch (gLink.SubState)
	{
	case LINK_SUB_STATE_WAIT_INIT_DONE:
		if (gLink.ToFlag)
		{
			DBGF;
			Link_Restart();
		}
		break;
	case LINK_SUB_STATE_WAIT_PIN:
		switch (gLink.PinCheck)
		{
		case LINK_CHECK_START:
			AT_AddReadCmd("CPIN", Link_PinCheckCB);
			gLink.PinCheck = LINK_CHECK_RUN;
			break;
		case LINK_CHECK_RUN:
			break;
		case LINK_CHECK_DONE:
			gLink.SubState = LINK_SUB_STATE_WAIT_CGATT;
			AT_AddRunCmd("GSN", Link_GetIMEICB);
			AT_AddRunCmd("CGDCONT=1,\"IP\",\"\"", Link_ATDummyCB);
			break;
		case LINK_CHECK_ERROR:
			Link_TimerStart(500);
			gLink.PinCheck = LINK_CHECK_WAIT;
			break;
		case LINK_CHECK_WAIT:
			if (gLink.ToFlag)
			{
				gLink.ToFlag = 0;
				AT_AddReadCmd("CPIN", Link_PinCheckCB);
				gLink.PinCheck = LINK_CHECK_RUN;
			}
			break;
		}
		break;
	case LINK_SUB_STATE_WAIT_CGATT:
		switch (gLink.AttachCheck)
		{
		case LINK_CHECK_START:
			AT_AddReadCmd("CGATT", Link_AttachCheckCB);
			gLink.AttachCheck = LINK_CHECK_RUN;
			break;
		case LINK_CHECK_RUN:
			break;
		case LINK_CHECK_DONE:
			gLink.SubState = LINK_SUB_STATE_WAIT_IP_OK;
			for (i = 0; i < sizeof(Link_SocketCmdQueue)/4; i++)
			{
				AT_AddRunCmd((int8_t *)Link_SocketCmdQueue[i], Link_ATDummyCB);
			}
			AT_AddRunCmd("CIFSR", Link_ActiveCB);
			break;
		case LINK_CHECK_ERROR:
			Link_TimerStart(AT_CGATT_PERIOD * 1000);
			gLink.AttachCheck = LINK_CHECK_WAIT;
			break;
		case LINK_CHECK_WAIT:
			if (gLink.ToFlag)
			{
				gLink.ToFlag = 0;
				AT_AddReadCmd("CGATT", Link_AttachCheckCB);
				gLink.AttachCheck = LINK_CHECK_RUN;
				if (!gSys.IMSI[0])
				{
					AT_AddRunCmd("CIMI", Link_GetIMSICB);
				}
			}
			break;
		}
		break;
	case LINK_SUB_STATE_WAIT_IP_OK:
		if (gLink.ActiveCheck)
		{
			DBG_INFO("active ok");
			gLink.MainState = LINK_MAIN_STATE_COMM;
			gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			gLink.IPState = LINK_IP_STATUS;
			Timer_Del(LINK_RESET_TIMER);
			Link_TimerStop();
		}
		break;
	default:
		DBGF;
		Link_Restart();
		break;
	}
}

static void Link_SubComm(void)
{
	int8_t Temp[64];
	Link_NotifyStruct Notify;

	switch (gLink.SubState)
	{
	case LINK_SUB_STATE_RUN_IDLE:
		if (gLink.ReqList.Len)
		{
			ReadRBuffer(&gLink.ReqList, &gLink.CurReq, 1);
			if (gLink.CurReq.SocketID >= SOCKET_MAX)
			{
				DBGF;
				break;
			}
			switch (gLink.CurReq.Req)
			{
			case LINK_REQ_CONNECT:
				sprintf(Temp, "%d,\"UDP\",\"%s\",\"%d\"", gLink.CurReq.SocketID, (int8_t *)gLink.CurReq.Data, gLink.CurReq.Len);
				gLink.SubState = LINK_SUB_STATE_CONNECT;
				AT_AddCmd("CIPSTART", Temp, AT_CMD_WRITE, 5000, Link_ATDummyCB);
				Link_TimerStart(AT_TCP_TO * 1000);
				break;
			case LINK_REQ_TX:
				sprintf(Temp, "%d,%d", gLink.CurReq.SocketID, gLink.CurReq.Len);
				gLink.SubState = LINK_SUB_STATE_TX;
				AT_AddCmd("CIPSEND", Temp, AT_CMD_WRITE, 5000, Link_ATDummyCB);
				gLink.TxSocketID = gLink.CurReq.SocketID;
				gLink.TxBuf = gLink.CurReq.Data;
				gLink.TxLen = gLink.CurReq.Len;
				Link_TimerStart(AT_TCP_TO * 1000);
				break;
			case LINK_REQ_CLOSE:
				sprintf(Temp, "%d", gLink.CurReq.SocketID);
				gLink.SubState = LINK_SUB_STATE_CLOSE;
				AT_AddCmd("CIPCLOSE", Temp, AT_CMD_WRITE, 5000, Link_ATDummyCB);
				Link_TimerStart(AT_TCP_TO * 1000);
				break;
			}
		}
		break;
	case LINK_SUB_STATE_CONNECT:
		if (gLink.ToFlag)
		{
			if (gLink.NotifyCB[gLink.CurReq.SocketID])
			{
				Notify.SocketID = gLink.CurReq.SocketID;
				Notify.RxData = NULL;
				Notify.RxDataLen = 0;
				Notify.Event = LINK_EVENT_CONNECT_FAIL;
				gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			}
			DBGF;
			Link_Restart();
		}
		break;
	case LINK_SUB_STATE_TX:
		if (gLink.ToFlag)
		{
			if (gLink.NotifyCB[gLink.CurReq.SocketID])
			{
				Notify.SocketID = gLink.CurReq.SocketID;
				Notify.RxData = NULL;
				Notify.RxDataLen = 0;
				Notify.Event = LINK_EVENT_TX_FAIL;
				gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			}
			DBGF;
			Link_Restart();
		}
		break;
	case LINK_SUB_STATE_CLOSE:
		if (gLink.ToFlag)
		{
			if (gLink.NotifyCB[gLink.CurReq.SocketID])
			{
				Notify.SocketID = gLink.CurReq.SocketID;
				Notify.RxData = NULL;
				Notify.RxDataLen = 0;
				Notify.Event = LINK_EVENT_CLOSE;
				gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			}
			DBGF;
			Link_Restart();
		}
		break;
	default:
		DBGF;
		Link_Restart();
		break;
	}
}

static void Link_SubSleep(void)
{
	DBGF;
	Link_Restart();
}

static void Link_SubInit(void)
{
	int i;
	switch (gLink.SubState)
	{
	case LINK_SUB_STATE_POWER_DOWN:
		if (gLink.ToFlag)
		{
			gLink.SubState = LINK_SUB_STATE_POWER_ON;
			IO_Write(PIN_IOT_POWER, 1);
			Link_TimerStart(10000);
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
		AT_AddDirCmd((int8_t *)Link_InitCmdQueue[0], Link_ATDummyCB);
		for (i = 1; i < sizeof(Link_InitCmdQueue)/4 - 1; i++)
		{
			AT_AddRunCmd((int8_t *)Link_InitCmdQueue[i], Link_ATDummyCB);
		}

		AT_AddRunCmd((int8_t *)Link_InitCmdQueue[i], Link_InitFinishCB);

		gLink.MainState = LINK_MAIN_STATE_ACTIVE;
		gLink.SubState = LINK_SUB_STATE_WAIT_INIT_DONE;
		Link_TimerStart(10000);
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
	gLink.PinCheck = 0;
	gLink.AttachCheck = 0;
	gLink.ActiveCheck = 0;
	gLink.IPState = LINK_IP_INITIAL;
	IO_Write(PIN_IOT_POWER, 0);
	Link_TimerStart(100);
	Timer_Start(LINK_RESET_TIMER, AT_RESET_TO * 1000, 0, Link_ResetTOCB);
}

void Link_Init(void)
{
	gSys.LinkCtrl = &gLink;
	InitRBuffer(&gLink.ReqList, gLink.ReqBuf, SOCKET_MAX, sizeof(Link_ReqStruct));
	Timer_Start(LINK_TIMER, 100, 0, Link_TOCB);
	Link_TimerStop();
	Link_Restart();
}

void Link_RxData(uint8_t *Data, uint16_t Len)
{
	Link_NotifyStruct Notify;
	if (gLink.RxSocketID < SOCKET_MAX)
	{
		if (gLink.NotifyCB[gLink.RxSocketID])
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
			if (gLink.SubState == LINK_SUB_STATE_CONNECT)
			{
				gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			}
			return ;
		}
		else if ( strstr(Str + 2, "CONNECT FAIL") )
		{
			Notify.Event = LINK_EVENT_CONNECT_FAIL;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			if (gLink.SubState == LINK_SUB_STATE_CONNECT)
			{
				gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			}
			return ;
		}
		else if ( strstr(Str + 2, "SEND OK") )
		{
			gLink.TxSocketID = 0xff;
			gLink.TxBuf = NULL;
			Notify.Event = LINK_EVENT_TX_OK;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			AT_FinishCmd();
			if (gLink.SubState == LINK_SUB_STATE_TX)
			{
				gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			}
			return ;
		}
		else if ( strstr(Str + 2, "SEND FAIL") )
		{
			gLink.TxSocketID = 0xff;
			gLink.TxBuf = NULL;
			Notify.Event = LINK_EVENT_TX_FAIL;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			AT_FinishCmd();
			if (gLink.SubState == LINK_SUB_STATE_TX)
			{
				gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			}
			return ;
		}
		else if ( strstr(Str + 2, "CLOSE OK") || strstr(Str + 2, "CLOSE FAIL") )
		{
			Notify.Event = LINK_EVENT_CLOSE;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			AT_FinishCmd();
			if (gLink.SubState == LINK_SUB_STATE_CLOSE)
			{
				gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			}
			return ;
		}
		else if ( strstr(Str + 2, "CLOSED") )
		{
			Notify.Event = LINK_EVENT_CLOSE;
			gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
			return ;
		}
	}
	else if (strstr(Str, AT_SOCKET_REC))
	{
		gLink.RxSocketID = 0xffffffff;
		sscanf(Str, "+RECEIVE,%d,%d:",&gLink.RxSocketID, &gLink.RxLen);
		DBG_INFO("%d, %d",gLink.RxSocketID, gLink.RxLen);
		if (gLink.RxSocketID != 0xffffffff)
		{
			AT_RxRawData(gLink.RxLen);
		}
	}
	else if (!strcmp(Str, AT_READY))
	{
		if (gLink.SubState == LINK_SUB_STATE_WAIT_READY)
		{
			DBG_INFO("link start init queue");
			Link_TimerStop();
			gLink.SubState = LINK_SUB_STATE_INIT_QUEUE;
		}
	}
	else if (!strcmp(Str, AT_PDP_DEACT))
	{
		DBGF;
		Link_Restart();
	}
	else if (!strcmp(Str, "+CFUN: 1"))
	{
		AT_AddRunCmd("CGATT=1", Link_ATDummyCB);
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

void Link_RegSocket(uint8_t SocketID, MyCBFun_t CB)
{
	if (SocketID < SOCKET_MAX)
	{
		gLink.NotifyCB[SocketID] = CB;
	}
}
void Link_AddReq(uint8_t SocketID, uint8_t ReqType, void *Data, uint16_t Len)
{
	Link_ReqStruct Req;
	if (SocketID < SOCKET_MAX)
	{
		Req.SocketID = SocketID;
		Req.Data = Data;
		Req.Len = Len;
		Req.Req = ReqType;
		WriteRBufferForce(&gLink.ReqList, &Req, 1);
	}
}

#endif
