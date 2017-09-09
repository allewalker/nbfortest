#ifndef __CAPI_H__
#define __CAPI_H__
#include <string.h>
#include <stdlib.h>
//#include "string.h"
//#include "stdlib.h"
#include "stdint.h"
#define INVALID_HANDLE_VALUE  ((void *)0xffffffff)
#define CRC32_GEN		(0x04C11DB7)
#define CRC16_CCITT_GEN		(0x1021)
#define CRC16_MODBUS_GEN		(0x8005)
#define CRC32_START		(0xffffffff)
#define CRC16_START		(0xffff)


#define IsAlpha(c)      (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')))
#define IsHex(c)      (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')))
#define IsDigit(c)        ((c >= '0') && (c <= '9'))
#define IsAlphaDigit(c)    (IsAlpha(c) || IsDigit(c))
#define BCD2HEX(x)	(((x >> 4) * 10) + (x & 0x0f))

typedef uint64_t LongInt;

typedef struct
{
	uint8_t *Data;
	uint32_t Pos;
	uint32_t MaxLen;
}Buffer_Struct;

typedef  struct
{
	uint32_t param_max_num;
	uint32_t param_max_len;
	uint32_t param_num;
	int8_t *param_str;
}CmdParam;

typedef struct
{
	uint8_t Sec;
	uint8_t Min;
	uint8_t Hour;
	uint8_t Week;//表示日期0~6,sun~sat，表示预约时，bit0~bit6,sun~sat
}Time_UserDataStruct;

typedef struct
{
	uint16_t Year;
	uint8_t Mon;
	uint8_t Day;
}Date_UserDataStruct;

typedef union
{
	uint32_t dwTime;
	Time_UserDataStruct Time;
}Time_Union;

typedef union
{
	uint32_t dwDate;
	Date_UserDataStruct Date;
}Date_Union;

typedef int32_t(*MyAPIFunc)(void *p);
typedef void(* MyCBFun_t)(void *pData);
typedef int (* MainFun_t)(void);
typedef struct
{
	uint32_t Cmd;
	MyAPIFunc Func;
}CmdFunStruct;

typedef struct
{
	uint8_t Cmd[24];
	MyAPIFunc Func;
}StrFunStruct;


typedef struct
{
	uint8_t *Data;
	uint32_t Len;
	uint32_t Offset;
	uint32_t MaxLength;
	uint32_t DataSize;
}RBuffer;

typedef struct
{
	double MeasureNoise_R; //测量误差
	double ProcessNoise_Q; //系统噪声
	double LastProcessResult_X; //上一个最优的系统状态值
	double LastCovariance_P; //上一个协方差
}KalmanFilter_Struct;

void UnicodeToAsciiN(uint16_t *Src, uint8_t *Dst, uint32_t Len);
void AsciiToUnicodeN(uint8_t *Src, uint16_t *Dst, uint32_t Len);
double AsciiToFloat(uint8_t *Src);
uint32_t AsciiToU32(uint8_t *Src, uint32_t Len);
uint32_t AsciiToHex(uint8_t *Src, uint32_t len, uint8_t *Dst);
uint32_t HexToAscii(uint8_t *Src, uint32_t Len, uint8_t *Dst);
uint32_t StrToUint(const uint8_t *Src);
void IntToBCD(uint32_t Src, uint8_t *Dst, uint8_t Len);
void LongToBCD(LongInt Src, uint8_t *Dst, uint8_t Len);
uint32_t BCDToInt(uint8_t *Src, uint8_t Len);
uint8_t IsDigitStr(const uint8_t *Src, uint32_t Len);
uint16_t AsciiToGsmBcd(int8_t *pNumber, uint8_t nNumberLen, uint8_t *pBCD);
void ReverseBCD(uint8_t *Src, uint8_t *Dst, uint32_t Len);

LongInt UTC2Tamp(Date_UserDataStruct *Date, Time_UserDataStruct *Time);
uint32_t Tamp2UTC(LongInt Sec, Date_UserDataStruct *Date, Time_UserDataStruct *Time, uint32_t LastDDay);
uint8_t XorCheck(void *Src, uint32_t Len, uint8_t CheckStart);
void CRC32_CreateTable(uint32_t *Tab, uint32_t Gen);
uint32_t CRC32_Cal(uint32_t * CRC32_Table, uint8_t *Buf, uint32_t Size, uint32_t CRC32Last);

uint16_t CRC16Cal(uint8_t *Src, uint16_t Len, uint16_t CRC16Last, uint16_t CRCRoot, uint8_t IsReverse);
uint32_t ReadRBuffer(RBuffer *Buf, void *Src, uint32_t Len);
uint32_t QueryRBuffer(RBuffer *Buf, void *Src, uint32_t Len);
void InitRBuffer(RBuffer *Buf, void *Src, uint32_t MaxLen, uint32_t DataSize);
void DelRBuffer(RBuffer *Buf, uint32_t Len);
uint32_t WriteRBufferForce(RBuffer *Buf, void *Src, uint32_t Len);

uint32_t TransferPack(uint8_t Flag, uint8_t Code, uint8_t F1, uint8_t F2, uint8_t *InBuf, uint32_t Len, uint8_t *OutBuf);
uint32_t TransferUnpack(uint8_t Flag, uint8_t Code, uint8_t F1, uint8_t F2, uint8_t *InBuf, uint32_t Len, uint8_t *OutBuf);

uint32_t CmdParseParam(int8_t* pStr, CmdParam *CmdParam, int8_t Cut);
double KalmanSingleFilter(KalmanFilter_Struct *Filter, double RealTimeValue);
#endif 

