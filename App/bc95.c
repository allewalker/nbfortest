#include "user.h"
#ifdef __NB_BC95__

#define AT_READY			"Neul "
#define AT_SOCKET_REC		"+NSONMI"
#define AT_RESET_TO			45
#define AT_TCP_TO			30
#define AT_CGATT_PERIOD		1
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
		"CGDCONT=1,\"IP\",\"\"",
		//"CFUN=1",
};

static Link_CtrlStruct gLink;

static void Link_TimerStart(uint32_t To);
static void Link_TimerStop();

static void Link_ATDummyCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Data)
	{
		Result->Data[Result->Size] = 0;
		DBG_INFO("%s", Result->Data);
	}
	else if (Result->Result)
	{
		DBG_INFO("%d", Result->Result);
	}
}

static void Link_GetIMEICB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	int8_t *Colon;
	if (Result->Data && Result->Size)
	{
		Result->Data[Result->Size] = 0;
		Colon = strchr(Result->Data, ':');
		if (Colon)
		{
			strcpy(gSys.IMEI, Colon+1);
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
			gLink.SubState = LINK_SUB_STATE_WAIT_CGATT;
			if (!gSys.IMEI[0])
			{
				AT_AddRunCmd("CGSN=1", Link_GetIMEICB);
			}
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

static void Link_AttachCheckCB(void *pData)
{
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	Link_TimerStop();
	if (Result->Data && Result->Size)
	{
		//DBG_INFO("%s", Result->Data);
		if (strstr(Result->Data, "+CGATT:1"))
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

static void Link_UDPCreateCB(void *pData)
{
	uint8_t Temp;
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Data)
	{
		if (IsDigit(Result->Data[0]))
		{
			Temp = Result->Data[0] - '0';
			if (Temp < SOCKET_MAX)
			{
				gLink.SocketBit |= 1 << Temp;
			}
		}
		DBG_INFO("%s", Result->Data);
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

static void Link_UDPTxCB(void *pData)
{
	Link_NotifyStruct Notify;
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	if (Result->Data)
	{
//		Result->Data[Result->Size] = 0;
//		DBG_INFO("%s", Result->Data);
	}
	else
	{
		Link_TimerStop();
		memset(&Notify, 0, sizeof(Notify));
		if ( (gLink.TxSocketID >= 0) && (gLink.TxSocketID < SOCKET_MAX) && (gLink.NotifyCB[gLink.TxSocketID]))
		{
			Notify.SocketID = gLink.TxSocketID;
			gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
			gLink.TxSocketID = 0xff;
			if (Result->Result)
			{
				DBG_INFO("%d", Result->Result);
				Notify.Event = LINK_EVENT_TX_FAIL;
			}
			else
			{
				Notify.Event = LINK_EVENT_TX_OK;
			}
			gLink.NotifyCB[Notify.SocketID](&Notify);
		}
		else
		{
			DBGF;
		}
	}
}

static void Link_UDPRxCB(void *pData)
{
	Link_NotifyStruct Notify;
	AT_ResultStruct *Result = (AT_ResultStruct *)pData;
	int8_t *Start, *End;
	if (Result->Data && Result->Size)
	{
		Result->Data[Result->Size] = 0;
		if (IsDigit(Result->Data[0]))
		{
			Notify.SocketID = Result->Data[0] - '0';
			if ( (Notify.SocketID < SOCKET_MAX) && gLink.NotifyCB[Notify.SocketID])
			{
				Start = strchr(Result->Data, ',');
				if (!Start)
				{
					DBGF;
					return;
				}
				Start = strchr(Start + 1, ',');
				if (!Start)
				{
					DBGF;
					return;
				}
				Start = strchr(Start + 1, ',');
				if (!Start)
				{
					DBGF;
					return;
				}
				Start = strchr(Start + 1, ',');
				if (!Start)
				{
					DBGF;
					return;
				}
				End = strchr(Start + 1, ',');
				if (!End)
				{
					DBGF;
					return;
				}
				*End = 0;
				Start++;
				Notify.RxDataLen = AsciiToHex(Start, strlen(Start), gLink.RxBuf);
				Notify.RxData = gLink.RxBuf;
				Notify.Event = LINK_EVENT_RX;
				gLink.NotifyCB[Notify.SocketID](&Notify);
			}
			else
			{
				DBG_ERR("%d", Notify.SocketID);
			}
		}
	}
	else if (Result->Result)
	{
		DBG_INFO("%d", Result->Result);
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
	int8_t Cmd[64];
	switch (gLink.SubState)
	{
	case LINK_SUB_STATE_WAIT_INIT_DONE:
		if (gLink.ToFlag)
		{
			DBGF;
			Link_Restart();
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
			Timer_Del(LINK_RESET_TIMER);
			for (i = 0;i < 6;i++)
			{
				sprintf(Cmd, "nsocr=DGRAM,17,%d", i + 10000);
				AT_AddRunCmd(Cmd, Link_UDPCreateCB);
			}
			gLink.SubState = LINK_SUB_STATE_WAIT_IP_OK;
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
		if (gLink.SocketBit == 0x3f)
		{
			gLink.MainState = LINK_MAIN_STATE_COMM;
			gLink.SubState = LINK_SUB_STATE_RUN_IDLE;
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
	uint16_t AT_TxLen = 0;
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

				if (gLink.NotifyCB[gLink.CurReq.SocketID])
				{
					gLink.IP[gLink.CurReq.SocketID] = (uint8_t *)gLink.CurReq.Data;
					gLink.Port[gLink.CurReq.SocketID] = gLink.CurReq.Len;
					Notify.Event = LINK_EVENT_CONNECT_OK;
					Notify.SocketID = gLink.CurReq.SocketID;
					Notify.RxData = NULL;
					gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
				}
				break;
			case LINK_REQ_TX:
				AT_TxLen = sprintf(gLink.TxBuf, "AT+NSOST=%d,%s,%d,%d,", gLink.CurReq.SocketID, gLink.IP[gLink.CurReq.SocketID], gLink.Port[gLink.CurReq.SocketID], gLink.CurReq.Len);
				AT_TxLen += HexToAscii((uint8_t *)gLink.CurReq.Data, gLink.CurReq.Len, &gLink.TxBuf[AT_TxLen]);
				gLink.TxBuf[AT_TxLen] = '\r';
				gLink.TxBuf[AT_TxLen + 1] = 0;
				gLink.SubState = LINK_SUB_STATE_TX;
				gLink.TxSocketID = gLink.CurReq.SocketID;
				AT_AddDirCmd(gLink.TxBuf, Link_UDPTxCB);
				Link_TimerStart(AT_TCP_TO * 1000);
				break;
			case LINK_REQ_CLOSE:
				if (gLink.NotifyCB[gLink.CurReq.SocketID])
				{
					Notify.Event = LINK_EVENT_CLOSE;
					Notify.SocketID = gLink.CurReq.SocketID;
					Notify.RxData = NULL;
					gLink.NotifyCB[Notify.SocketID]((void *)&Notify);
				}
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
		if (gLink.ToFlag)
		{
			AT_AddDirCmd((int8_t *)Link_InitCmdQueue[0], Link_ATDummyCB);
			for (i = 1; i < sizeof(Link_InitCmdQueue)/4 - 1; i++)
			{
				AT_AddRunCmd((int8_t *)Link_InitCmdQueue[i], Link_ATDummyCB);
			}
			AT_AddCmd((int8_t *)Link_InitCmdQueue[i], NULL, AT_CMD_RUN, 10 * 1000, Link_InitFinishCB);
			gLink.MainState = LINK_MAIN_STATE_ACTIVE;
			gLink.SubState = LINK_SUB_STATE_WAIT_INIT_DONE;
			Link_TimerStart(10000);
		}
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
	int i;
	gLink.MainState = LINK_MAIN_STATE_INIT;
	gLink.SubState = LINK_SUB_STATE_POWER_DOWN;
	gLink.SocketBit = 0;
	gLink.AttachCheck = 0;
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

}

void Link_TxData(void)
{

}

void Link_UrcAnalyze(int8_t *Str)
{

	uint32_t RxSocketID, RxLen;
	uint8_t Cmd[64];
	if (strstr(Str, AT_SOCKET_REC))
	{
		DBG_INFO("%d need read %dbyte", RxSocketID, RxLen);
		sprintf(Cmd, "NSORF=%d,%d", RxSocketID, RxLen);
		AT_AddRunCmd(Cmd, Link_UDPRxCB);
	}
	else if (!strcmp(Str, AT_READY))
	{
		if (gLink.SubState == LINK_SUB_STATE_WAIT_READY)
		{
			DBG_INFO("link start init queue");
			Link_TimerStart(50);
			gLink.SubState = LINK_SUB_STATE_INIT_QUEUE;
		}
	}
	else if (!strcmp(Str, "+CFUN: 1"))
	{
		AT_AddRunCmd("CGATT=1", Link_ATDummyCB);
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
