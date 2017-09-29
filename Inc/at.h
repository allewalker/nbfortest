#ifndef __AT_H__
#define __AT_H__
enum
{
	AT_CMD_NONE,	//无AT命令
	AT_CMD_TEST,	//测试命令，一般用于获取参数
	AT_CMD_READ,
	AT_CMD_WRITE,
	AT_CMD_RUN,
	AT_CMD_DIR,
};

typedef struct
{
	int32_t Result;
	int8_t *Data;	//返回的数据，如果有':'，则返回':'之后的数据
	uint32_t Size;
}AT_ResultStruct;

#define AT_LINK_SHUT_OK			"SHUT OK"
#define AT_RESULT_OK			"OK"
#define AT_RESULT_ERROR			"ERROR"
#define AT_RESULT_CME			"+CME ERROR"
#define AT_RESULT_CMS			"+CMS ERROR"
void AT_Init(void);
/*
 * Cmd:after'+',before'='
 * Param:after'='
 * Type:TEST .. RUN
 * To:ms
 * CB:callback
 */
void AT_AddCmd(int8_t *Cmd, int8_t *Param, uint8_t Type, uint32_t To, MyCBFun_t CB);
void AT_AddReadCmd(int8_t *Cmd, MyCBFun_t CB);
void AT_AddRunCmd(int8_t *Cmd, MyCBFun_t CB);
void AT_Task(void *Param);
void AT_SendRawData(uint8_t *Data, uint16_t Len);
void AT_FinishCmd(void);
void AT_Reset(void);
#endif
