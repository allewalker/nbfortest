#ifndef __DEBUG_H__
#define __DEBUG_H__
#define DBG_BUF_SIZE (256)

#if 1
#define DBG_INFO(x,y...) DBG_Printf("%s %d:"x"\r\n", __FUNCTION__,__LINE__,##y)
#define DBG_ERR(x,y...) DBG_Printf("%s %d:"x"\r\n", __FUNCTION__,__LINE__,##y)
#define DBG(x,y...) DBG_Printf(x, ##y)
#define DBGF	DBG_INFO("!\r\n")
#else
#define DBG_FUNC(x,y...)
#define DBG_INFO(x,y...)
#define DBG_ERR(x,y...)
#define DBG
#endif
typedef struct
{
	unsigned char *RemoteData;
	unsigned int RemoteLen;
	unsigned int ReceiveLen;
	unsigned int DataLen;
	unsigned int Cmd;
	unsigned char State;
	unsigned char Count;
	unsigned char ReceiveFlag;
	unsigned char DirSendFlag;
}DBG_ProtocolStruct;

void DBG_Config(void);
void DBG_HexPrintf(unsigned char *data, unsigned int len);
void DBG_Printf(const char* format, ...);
int32_t DBG_Send(void);
void DBG_UsartReceive(uint8_t data);
int DBG_GetData(void *Data, unsigned int Len);
int DBG_ReturnBufLen(void);
#endif
