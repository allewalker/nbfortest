#include "user.h"
#define AT_RX_LEN	(1600)
#define AT_DMA_LEN	(128)
#define AT_TX_LEN	(32)
#define AT_RESULT_LEN	(128)
#define AT_UART_TO		(10)
enum
{
	LLPARSE_STATE_IDLE,		/* idle, not parsing a response */
	LLPARSE_STATE_IDLE_CR,		/* CR before response (V1) */
	LLPARSE_STATE_IDLE_LF,		/* LF before response (V1) */
	LLPARSE_STATE_RESULT,		/* within result payload */
	LLPARSE_STATE_RESULT_CR,	/* CR after result */
	LLPARSE_STATE_RESULT_LF,	/* LF after result */
	LLPARSE_STATE_PROMPT,		/* within a "> " prompt */
	LLPARSE_STATE_ERROR,		/* something went wrong */
	LLPARSE_STATE_RAWDATA,  /*for read rawdata from uart*/
};

typedef struct
{
	int8_t *Str;
	void (*Fun)(int8_t *Data);
}AT_UrcStruct;

typedef struct
{
	uint32_t To;
	uint8_t CmdStr[32];
	uint8_t Param[64];
	uint8_t TxType;
	uint8_t pad[3];
	MyCBFun_t cb;
}AT_TxStruct;

typedef struct
{
	uint8_t ResultBuf[AT_RESULT_LEN];
	uint32_t ResultCnt;
	uint8_t Payload[AT_RX_LEN];
	uint16_t PayloadCnt;
	uint16_t PayloadMax;			//socket/sms 需要接收的长度
	uint8_t TxDMABuf[AT_DMA_LEN];
	uint8_t RxDMABuf[AT_RX_LEN];	//本次接收的字节
	uint8_t AnalyzeBuf[AT_RX_LEN];	//需要解析的数据区
	uint8_t AnalyzeState;
	uint8_t NewAnalyzeFlag;
	uint16_t AnalyzeSize;
	uint16_t RxSize;				//本次接收的长度
	RBuffer TxQueue;				//AT指令缓存队列
	AT_TxStruct TxBuf[AT_TX_LEN];	//AT指令缓存区
	AT_TxStruct CurCmd;				//当前的AT指令
}AT_CtrlStruct;

static AT_CtrlStruct gAT;

static void AT_UartTimerCB(void *pData)
{
	uint16_t CopySize;
	uint16_t RxSize = (AT_RX_LEN - 1 - Uart_RxDMAGetSize(AT_UART_ID));
	if (RxSize != gAT.RxSize)
	{
		gAT.RxSize = RxSize;
	}
	else
	{
		Timer_Switch(AT_RX_TIMER, 0);
		gAT.RxSize++;
		Uart_RxDMAStop(AT_UART_ID);
		if (RxSize <= AT_RX_LEN)
		{
			CopySize = RxSize;
		}
		else
		{
			DBG_ERR("overflow,%d", RxSize);
			CopySize = AT_RX_LEN;
		}
		memcpy(&gAT.AnalyzeBuf, gAT.RxDMABuf, CopySize);
		gAT.AnalyzeSize = CopySize;
		gAT.NewAnalyzeFlag = 1;
		gAT.AnalyzeBuf[gAT.AnalyzeSize] = 0;
		if (gAT.AnalyzeSize < 64)
		{
			DBG("AT RX: %s\r\n", gAT.AnalyzeBuf);
		}
	}
}

static void AT_UartCB(void *pData)
{
	uint32_t Type = (uint32_t)pData;
	if (Type == UART_RX_IRQ)
	{
		gAT.RxDMABuf[0] = Uart_Rx(UART_ID2);
		gAT.RxSize = 0;
		Uart_RxDMAStart(AT_UART_ID, gAT.RxDMABuf + 1, AT_RX_LEN - 1);
		Timer_Restart(AT_RX_TIMER, AT_UART_TO);
	}
}

static void AT_RunToCB(void *pData)
{
	AT_ResultStruct Result;
	MyCBFun_t CB;
	DBG_ERR("%s run To", gAT.CurCmd.CmdStr);
	Timer_Del(AT_RUN_TIMER);
	//在这里添加重启模块的代码
	Uart_Switch(AT_UART_ID, 0);
	//并对剩余所有命令都发出超时失败的响应
	Result.Result = ERROR_AT_CMD_TO;
	Result.Size = 0;
	if (gAT.CurCmd.cb)
	{
		gAT.CurCmd.cb((void *)&Result);
	}
	else
	{
		memset(&gAT.CurCmd, 0, sizeof(gAT.CurCmd));
	}

	while(gAT.TxQueue.Len)
	{
		ReadRBuffer(&gAT.TxQueue, &gAT.CurCmd, 1);
		if (gAT.CurCmd.cb)
		{
			gAT.CurCmd.cb((void *)&Result);
		}
	}

	Uart_Switch(AT_UART_ID, 1);
	AT_Reset();
}

static void AT_SendCurCmd(void)
{
	uint16_t TxLen;
	switch (gAT.CurCmd.TxType)
	{
	case AT_CMD_TEST:
		TxLen = snprintf(gAT.TxDMABuf, AT_DMA_LEN, "AT+%s=?\r", gAT.CurCmd.CmdStr);
		break;
	case AT_CMD_READ:
		TxLen = snprintf(gAT.TxDMABuf, AT_DMA_LEN, "AT+%s?\r", gAT.CurCmd.CmdStr);
		break;
	case AT_CMD_WRITE:
		TxLen = snprintf(gAT.TxDMABuf, AT_DMA_LEN, "AT+%s=%s\r", gAT.CurCmd.CmdStr, gAT.CurCmd.Param);
		break;
	case AT_CMD_RUN:
		TxLen = snprintf(gAT.TxDMABuf, AT_DMA_LEN, "AT+%s\r", gAT.CurCmd.CmdStr);
		break;
	case AT_CMD_DIR:
		TxLen = snprintf(gAT.TxDMABuf, AT_DMA_LEN, "AT%s\r", gAT.CurCmd.CmdStr);
		break;
	default:
		gAT.CurCmd.TxType = AT_CMD_NONE;
		return;
	}
	if (TxLen < 64)
	{
		DBG("AT TX: %s\r\n", gAT.TxDMABuf);
	}
	Uart_Tx(AT_UART_ID, gAT.TxDMABuf, TxLen);
	if (gAT.CurCmd.To < 100)
	{
		Timer_Start(AT_RUN_TIMER, 100, 0, AT_RunToCB);
	}
	else
	{
		Timer_Start(AT_RUN_TIMER, gAT.CurCmd.To, 0, AT_RunToCB);
	}
}

static void AT_AnalyzeLineDone(void)
{
	gAT.ResultCnt = 0;
	gAT.AnalyzeState = LLPARSE_STATE_IDLE;
}

/*
 * 从AT响应中读取1个字节，并更新读取的状态
 */
static int32_t AT_AnalyzeByte(uint8_t Byte)
{
    int32_t ret = 0;

    //oa_trace_on_file("b=%02X, %d, %d, %c", byte, llp->state, llp->raw_data_count, byte);

    switch (gAT.AnalyzeState)
    {
    case LLPARSE_STATE_IDLE:
        if (Byte == '\r')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_IDLE_CR;
        }
        else if (Byte == '>')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_PROMPT;
        }
        else if (Byte == '\n')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT_LF;
        }
        else
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT;
            gAT.ResultBuf[0] = Byte;
            gAT.ResultCnt = 1;
        }
        break;

    case LLPARSE_STATE_IDLE_CR:
        if (Byte == '\n')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT_LF;
        }
        else if(Byte == '\r')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_IDLE_CR;
        }
        else
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT;
            gAT.ResultBuf[0] = Byte;
            gAT.ResultCnt = 1;
        }
        break;
    case LLPARSE_STATE_RESULT_CR:
    	AT_AnalyzeLineDone();
        if (Byte == '\n')
        {
        	if (gAT.PayloadMax)
        	{
        		gAT.AnalyzeState = LLPARSE_STATE_RESULT_LF;
        	}
        	else
        	{
        		gAT.AnalyzeState = LLPARSE_STATE_IDLE;
        	}
        }
        else if(Byte == '\r')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_IDLE_CR;
        }
        else if(Byte == '>')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_PROMPT;
        }
        else
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT;
            gAT.ResultBuf[0] = Byte;
            gAT.ResultCnt = 1;
        }
    	break;
    case LLPARSE_STATE_RESULT_LF:
    	AT_AnalyzeLineDone();
        if(gAT.PayloadMax)
        {
            gAT.Payload[0] = Byte;
            gAT.PayloadCnt = 1;
            gAT.AnalyzeState = LLPARSE_STATE_RAWDATA;
        }
        else
        {
            if(Byte == '\r')
            {
            	gAT.AnalyzeState = LLPARSE_STATE_IDLE_CR;
            }
            else if(Byte == '>')
            {
            	gAT.AnalyzeState = LLPARSE_STATE_PROMPT;
            }
            else
            {
            	gAT.AnalyzeState = LLPARSE_STATE_RESULT;
                gAT.ResultBuf[0] = Byte;
                gAT.ResultCnt = 1;
            }
        }
        break;

    case LLPARSE_STATE_RESULT:
        if (Byte == '\r')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT_CR;
        }
        else
        {
        	gAT.ResultBuf[gAT.ResultCnt] = Byte;
        	gAT.ResultCnt++;
        }
        break;

    case LLPARSE_STATE_RAWDATA:
        if(gAT.PayloadCnt >= gAT.PayloadMax )
        {
        	AT_AnalyzeLineDone();
            if(Byte == '\r') {
            	gAT.AnalyzeState = LLPARSE_STATE_IDLE_CR;
            } else if(Byte == '>') {
            	gAT.AnalyzeState = LLPARSE_STATE_PROMPT;
            } else {
            	gAT.AnalyzeState = LLPARSE_STATE_RESULT;
            	DBG_ERR("overflow raw data!");
                gAT.ResultBuf[0] = Byte;
                gAT.ResultCnt = 1;
            }
        }
        else
        {
            gAT.Payload[gAT.PayloadCnt] = Byte;
            gAT.PayloadCnt++;
        }
        break;

    case LLPARSE_STATE_PROMPT:
    	gAT.ResultCnt = 0;
        if (Byte == '\r')
        {
        	gAT.AnalyzeState = LLPARSE_STATE_IDLE_CR;
        }
        else
        {
        	gAT.AnalyzeState = LLPARSE_STATE_RESULT;
            gAT.ResultBuf[0] = Byte;
            gAT.ResultCnt = 1;
        }
        break;

    default:
        /*fatal error!*/
    	ret = ERROR_AT_ANALYZE_STATE;
        break;
    }
    //DBG("%d\r\n", gAT.AnalyzeState);
    return ret;
}

static void AT_AnalyzeResult(void)
{

	int8_t *Colon;
	AT_ResultStruct Result;
	if (!gAT.ResultCnt)
	{
		return ;
	}

	if (gAT.ResultCnt < AT_RESULT_LEN)
	{
		gAT.ResultBuf[gAT.ResultCnt] = 0;
	}
	else
	{
		gAT.ResultBuf[AT_RESULT_LEN - 1] = 0;
	}
	Colon = strchr(gAT.ResultBuf, ':');
	Result.Result = 0;
	if (Colon)
	{
		Result.Data = Colon + 1;
		Result.Size = (uint32_t)gAT.ResultBuf + AT_RESULT_LEN - 1 - (uint32_t)Colon;
	}
//	else
//	{
//		Result.Data = gAT.ResultBuf;
//		Result.Size = gAT.ResultCnt;
//	}
	//如果有回调函数，根据当前正在执行的命令类型进行相应处理
	if (gAT.CurCmd.cb)
	{
		Result.Data = gAT.ResultBuf;
		Result.Size = gAT.ResultCnt;
		switch (gAT.CurCmd.TxType)
		{
		case AT_CMD_READ:
		case AT_CMD_WRITE:
		case AT_CMD_DIR:
			/*通过和发送的比较确认是不是当前命令的返回，如果是，则交给回调函数*/
			if (!memcmp(&gAT.ResultBuf[1], gAT.CurCmd.CmdStr, strlen(gAT.CurCmd.CmdStr)))
			{
				//DBG_INFO("%s", gAT.ResultBuf);
				gAT.CurCmd.cb((void *)&Result);
				return;
			}
			break;
		case AT_CMD_RUN:
			/*如果是运行的指令，可能不返回‘+’，需要进入回调函数进行判断*/
			gAT.CurCmd.cb((void *)&Result);
			break;
		}

	}

	//检查是否为结束的响应
	if (Colon)
	{
		if (!memcmp(gAT.ResultBuf, AT_RESULT_CME, 10) || !memcmp(gAT.ResultBuf, AT_RESULT_CMS, 10))
		{
			Result.Data = NULL;
			Result.Result = strtoul(Colon+1, NULL, 10) + 1;
			if (gAT.CurCmd.TxType && gAT.CurCmd.cb)
			{
				gAT.CurCmd.cb((void *)&Result);
			}
			AT_FinishCmd();
			return;
		}
	}
	else
	{
        if (!strcmp(gAT.ResultBuf, AT_RESULT_ERROR))
        {
             /*Part of Case 'C'*/
        	Result.Result = 1;
        	Result.Data = NULL;
			if (gAT.CurCmd.TxType && gAT.CurCmd.cb)
			{
				gAT.CurCmd.cb((void *)&Result);
			}
			AT_FinishCmd();
			return;
        }

        if (!strcmp(gAT.ResultBuf, AT_RESULT_OK) || !strcmp(gAT.ResultBuf, AT_LINK_SHUT_OK))
        {
             /*Part of Case 'C'*/
        	Result.Result = 0;
        	Result.Data = NULL;
			if (gAT.CurCmd.TxType && gAT.CurCmd.cb)
			{
				gAT.CurCmd.cb((void *)&Result);
			}
			AT_FinishCmd();
			return;
        }
	}

	//最后进入主动上报或者其他固定的处理
	Link_UrcAnalyze(gAT.ResultBuf);
}

void AT_Reset(void)
{
	gAT.AnalyzeSize = 0;
	gAT.AnalyzeState = LLPARSE_STATE_IDLE;
	gAT.ResultCnt = 0;
	gAT.PayloadCnt = 0;
	gAT.PayloadMax = 0;
	memset(&gAT.CurCmd, 0, sizeof(gAT.CurCmd));
}

void AT_FinishCmd(void)
{
	DBG_INFO("%s done", gAT.CurCmd.CmdStr);
	Timer_Del(AT_RUN_TIMER);
	memset(&gAT.CurCmd, 0, sizeof(gAT.CurCmd));
	if (gAT.TxQueue.Len)	//当前没有AT指令
	{
		ReadRBuffer(&gAT.TxQueue, &gAT.CurCmd, 1);
		AT_SendCurCmd();
	}
}

void AT_Init(void)
{
	InitRBuffer(&gAT.TxQueue, gAT.TxBuf, AT_TX_LEN, sizeof(AT_TxStruct));
	Uart_Config(AT_UART_ID, AT_BR, AT_UartCB, 1);
	Uart_RxDMAInit(AT_UART_ID);
	Timer_Start(AT_RX_TIMER, AT_UART_TO, 1, AT_UartTimerCB);
	Timer_Switch(AT_RX_TIMER, 0);
	AT_Reset();
}

void AT_AddReadCmd(int8_t *Cmd, MyCBFun_t CB)
{
	AT_AddCmd(Cmd, NULL ,AT_CMD_READ, 200, CB);
}

void AT_AddRunCmd(int8_t *Cmd, MyCBFun_t CB)
{
	AT_AddCmd(Cmd, NULL ,AT_CMD_RUN, 200, CB);
}

void AT_AddDirCmd(int8_t *Cmd, MyCBFun_t CB)
{
	AT_AddCmd(Cmd, NULL ,AT_CMD_DIR, 200, CB);
}

void AT_AddCmd(int8_t *Cmd, int8_t *Param, uint8_t Type, uint32_t To, MyCBFun_t CB)
{
	AT_TxStruct Tx;
	if (!Cmd || !CB)
	{
		return;
	}
	memset(&Tx, 0, sizeof(Tx));
	strncpy(Tx.CmdStr, Cmd, sizeof(Tx.CmdStr) - 1);

	if (Param)
	{
		strncpy(Tx.Param, Param, sizeof(Tx.Param) - 1);
	}

	Tx.TxType = Type;
	Tx.To = To;
	Tx.cb = CB;
	if (!gAT.CurCmd.TxType && !gAT.TxQueue.Len)	//当前没有AT指令，也没有缓存的指令
	{
		gAT.CurCmd = Tx;
		AT_SendCurCmd();
	}
	else
	{
		WriteRBufferForce(&gAT.TxQueue, &Tx, 1);
	}
}

void AT_Task(void *Param)
{
	uint16_t i;
	if (gAT.NewAnalyzeFlag)
	{
		gAT.NewAnalyzeFlag = 0;
		for(i = 0;i < gAT.AnalyzeSize;i++)
		{
			if ( AT_AnalyzeByte(gAT.AnalyzeBuf[i]) < 0)
			{
				//请求重启模块;
				DBG_ERR("!");
				Link_Restart();
				AT_Reset();
				return;
			}
			if (gAT.ResultCnt >= AT_RESULT_LEN)
			{
				DBG_ERR("result data overflow!");
				//对结果进行解析
				AT_AnalyzeResult();
				AT_AnalyzeLineDone();

			}
			if(gAT.AnalyzeState == LLPARSE_STATE_RESULT_CR)
			{
				//对结果进行解析
				AT_AnalyzeResult();
			}

			if( (gAT.AnalyzeState == LLPARSE_STATE_RAWDATA) && (gAT.PayloadCnt >= gAT.PayloadMax) )
			{
				//数据根据SMS还是SOCKET的，交给link
				Link_RxData(gAT.Payload, gAT.PayloadCnt);
				//解析完，解析状态恢复到IDLE
				AT_AnalyzeLineDone();
				gAT.PayloadMax = 0;
			}

			if(gAT.AnalyzeState == LLPARSE_STATE_PROMPT)
			{
				//数据根据SMS还是SOCKET的，发送原始数据
				Link_TxData();
			}
		}
	}
	if( (gAT.AnalyzeState == LLPARSE_STATE_RAWDATA) && (gAT.PayloadCnt == (gAT.PayloadMax - 1)) )
	{
		//数据根据SMS还是SOCKET的，交给link
		DBGF;
		Link_RxData(gAT.Payload, gAT.PayloadCnt);
		//解析完，解析状态恢复到IDLE
		AT_AnalyzeLineDone();
		gAT.PayloadMax = 0;
	}
RX_ANALYZE_DONE:
	if (!gAT.CurCmd.TxType && gAT.TxQueue.Len)	//当前没有AT指令
	{
		ReadRBuffer(&gAT.TxQueue, &gAT.CurCmd, 1);
		AT_SendCurCmd();
	}
}

void AT_TxRawData(uint8_t *Data, uint16_t Len)
{
	Uart_Tx(AT_UART_ID, Data, Len);
}

void AT_RxRawData(uint16_t len)
{
    gAT.PayloadMax = len;
    gAT.PayloadCnt = 0;
}
