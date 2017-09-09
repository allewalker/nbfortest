#include "CApi.h"


/*****************************************************************************
* FUNCTION
*   command_parse_param()
* DESCRIPTION
*    Parse AT command string to parameters
* PARAMETERS
*   char* pStr
* RETURNS
*  pCmdParam
*****************************************************************************/
uint32_t CmdParseParam(int8_t* pStr, CmdParam *CP, int8_t Cut)
{
	uint32_t paramStrLen = strlen((char *)pStr);
	uint32_t paramIndex = 0;
	uint32_t paramCharIndex = 0;
	uint32_t index = 0;

	while ((pStr[index] != '\r')
		&& (index < paramStrLen)
		&& (paramIndex < CP->param_max_num)) {
		if (pStr[index] == Cut) {
			/* Next param string */
			paramCharIndex = 0;
			paramIndex++;
		}
		else {
			if (pStr[index] != '"')
			{
				if (paramCharIndex >= CP->param_max_len)
					return (0);

				/*Get each of command param char, the param char except char ' " '*/
				CP->param_str[paramIndex * CP->param_max_len + paramCharIndex] = pStr[index];
				paramCharIndex++;
			}
		}
		index++;
	}

	CP->param_num = paramIndex + 1;

	return (1);
}


void UnicodeToAsciiN(uint16_t *Src, uint8_t *Dst, uint32_t Len)
{
	uint32_t i;
	for (i = 0; i < Len; i++)
	{
		Dst[i] = Src[i];
	}
}

void AsciiToUnicodeN(uint8_t *Src, uint16_t *Dst, uint32_t Len)
{
	uint32_t i;
	for (i = 0; i < Len; i++)
	{
		Dst[i] = Src[i];
		if (!Src[i])
			break;
	}
}

uint8_t IsDigitStr(const uint8_t *Src, uint32_t Len)
{
	uint32_t i = 0;
	while (i < Len){
		if (!IsDigit(Src[i])) {
			return 0;
		}
		i++;
	}
	return 1;
}

void ReverseBCD(uint8_t *Src, uint8_t *Dst, uint32_t Len)
{
	uint32_t i;
	for (i = 0; i < Len; i++)
	{
		Dst[i] = (Src[i] >> 4) | (Src[i] << 4);
	}
}

uint16_t AsciiToGsmBcd(int8_t *pNumber, uint8_t nNumberLen, uint8_t *pBCD)
{
	uint8_t Tmp;
	uint32_t i;
	uint32_t nBcdSize = 0;
	uint8_t *pTmp     = pBCD;

	if (!pNumber|| !pBCD)
		return 0;

	memset(pBCD, 0, nNumberLen >> 1);

	for (i = 0; i < nNumberLen; i++)
	{
		switch (*pNumber)
		{

		case '*':
			Tmp = (int8_t)0x0A;
			break;

		case '#':
			Tmp = (int8_t)0x0B;
			break;

		case 'p':
			Tmp = (int8_t)0x0C;
			break;

		default:

			if (IsDigit((*pNumber)))
			{

				Tmp = (int8_t)(*pNumber - '0');
			}
			else
			{
				return 0;
			}
			break;
		}

		if (i % 2)
		{
			*pTmp++ |= (Tmp << 4) & 0xF0;
		}
		else
		{
			*pTmp = (int8_t)(Tmp & 0x0F);
		}

		pNumber++;
	}

	nBcdSize = nNumberLen >> 1;

	if (i % 2)
	{
		*pTmp |= 0xF0;
		nBcdSize += 1;
	}

	return nBcdSize;
}

uint32_t AsciiToHex(uint8_t *Src, uint32_t Len, uint8_t *Dst)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint8_t hex_temp;
	while (i < Len) {
		hex_temp = 0;
		if (IsDigit(Src[i]))
		{
			hex_temp = Src[i++] - '0';
		}
		else if (Src[i] > 'Z')
		{
			hex_temp = Src[i++] - 'a' + 10;
		}
		else
		{
			hex_temp = Src[i++] - 'A' + 10;
		}
		hex_temp <<= 4;
		if (IsDigit(Src[i]))
		{
			hex_temp += Src[i++] - '0';
		}
		else if (Src[i] > 'Z')
		{
			hex_temp += Src[i++] - 'a' + 10;
		}
		else
		{
			hex_temp += Src[i++] - 'A' + 10;
		}
		Dst[j++] = hex_temp;
	}
	return j;
}

uint32_t AsciiToU32(uint8_t *Src, uint32_t Len)
{
	uint32_t i = 0;
	uint32_t Temp = 0;
	for (i = 0; i < Len; i++)
	{

		if (Src[i])
		{
			Temp *= 10;
			Temp += Src[i] - '0';
		}
		else
		{
			break;
		}
	}
	return Temp;
}

double AsciiToFloat(uint8_t *Src)
{

	double s=0.0;

	double d=10.0;
	int jishu=0;

	uint8_t flag=0;

	while(*Src==' ')
	{
		Src++;
	}

	if(*Src=='-')//记录数字正负
	{
		flag=1;
		Src++;
	}

	if(!(*Src>='0'&&*Src<='9'))//如果一开始非数字则退出，返回0.0
		return s;

	while(*Src>='0'&&*Src<='9'&&*Src!='.')//计算小数点前整数部分
	{
		s=s*10.0+*Src-'0';
		Src++;
	}

	if(*Src=='.')//以后为小树部分
		Src++;

	while(*Src>='0'&&*Src<='9')//计算小数部分
	{
		s=s+(*Src-'0')/d;
		d*=10.0;
		Src++;
	}

	if(*Src=='e'||*Src=='E')//考虑科学计数法
	{
		Src++;
		if(*Src=='+')
		{
			Src++;
			while(*Src>='0'&&*Src<='9')
			{
				jishu=jishu*10+*Src-'0';
				Src++;
			}
			while(jishu>0)
			{
				s*=10;
				jishu--;
			}
		}
		if(*Src=='-')
		{
			Src++;
			while(*Src>='0'&&*Src<='9')
			{
				jishu=jishu*10+*Src-'0';
				Src++;
			}
			while(jishu>0)
			{
				s/=10;
				jishu--;
			}
		}
	}

	return s*(flag?-1.0:1.0);

}

void IntToBCD(uint32_t Src, uint8_t *Dst, uint8_t Len)
{
	uint8_t i, j, k;
	uint8_t Temp[64];
	memset(Dst, 0, Len);
	memset(Temp, 0, 64);
	j = 0;
	while (Src)
	{
		Temp[j] = Src % 10;
		Src = Src/10;
		j++;
	}
	k = 0;
	for (i = Len; i > 0; i--)
	{
		Dst[i - 1] = Temp[k] + (Temp[k + 1] << 4);
		k += 2;
		if (k >= j)
		{
			break;
		}
	}
}

void LongToBCD(LongInt Src, uint8_t *Dst, uint8_t Len)
{
	uint8_t i, j, k;
	uint8_t Temp[64];
	memset(Dst, 0, Len);
	memset(Temp, 0, 64);
	j = 0;
	while (Src)
	{
		Temp[j] = Src % 10;
		Src = Src/10;
		j++;
	}
	k = 0;
	for (i = Len; i > 0; i--)
	{
		Dst[i - 1] = Temp[k] + (Temp[k + 1] << 4);
		k += 2;
		if (k >= j)
		{
			break;
		}
	}
}

uint32_t BCDToInt(uint8_t *Src, uint8_t Len)
{
	uint32_t Result = 0;
	uint8_t Temp, i;
	for (i = 0; i < Len; i++)
	{
		Result *= 100;
		Temp = (Src[i] >> 4) * 10 + (Src[i] & 0x0f);
		Result += Temp;
	}
	return Result;
}

uint32_t HexToAscii(uint8_t *Src, uint32_t Len, uint8_t *Dst)
{
	uint32_t i = 0;
	uint32_t j = 0;
	uint8_t hex_temp;
	while (i < Len) {
		hex_temp = (Src[i] & 0xf0) >> 4;
		if (hex_temp >= 10) {
			Dst[j++] = hex_temp - 10 + 'A';
		}
		else {
			Dst[j++] = hex_temp + '0';
		}
		hex_temp = Src[i++] & 0x0f;
		if (hex_temp >= 10) {
			Dst[j++] = hex_temp - 10 + 'A';
		}
		else {
			Dst[j++] = hex_temp + '0';
		}

	}
	return j;
}

uint32_t StrToUint(const uint8_t *Src)
{
	uint32_t hex_temp = 0;
	uint32_t i;
	for (i = 0; i < 8; i++)
	{
		if (Src[i])
		{
			if (i)
			{
				hex_temp <<= 4;
			}
			if (IsDigit(Src[i]))
			{
				hex_temp += Src[i] - '0';
			}
			else if (Src[i] >= 'a' && Src[i] <= 'f')
			{
				hex_temp += Src[i] - 'a' + 10;
			}
			else if (Src[i] >= 'A' && Src[i] <= 'Z')
			{
				hex_temp += Src[i] - 'A' + 10;
			}
			else
			{
				return 0;
			}

		}
		else
		{
			return hex_temp;
		}
	}
	return hex_temp;
}

/************************************************************************/
/*时间与时间戳转换，C语言实现                                                                    */
/************************************************************************/
uint8_t IsLeapYear(uint32_t Year)
{
	if ((((Year % 4) == 0) && ((Year % 100) != 0)) || ((Year % 400) == 0))
		return 1;
	else
		return 0;
}

LongInt UTC2Tamp(Date_UserDataStruct *Date, Time_UserDataStruct *Time)
{
	uint32_t DayTable[2][12] = { { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 }, { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 } };
	LongInt DYear, DDay, DSec;

	DYear = Date->Year - 1970;
	if (DYear)	//1970年以后,1972是第一个闰年
	{
		DDay = DYear * 365 + ((DYear + 2) / 4) - ((DYear + 2) / 100) + ((DYear + 2) / 400) + DayTable[IsLeapYear(Date->Year)][Date->Mon - 1] + (Date->Day - 1);
	}
	else
	{
		DDay = DayTable[IsLeapYear(Date->Year)][Date->Mon - 1] + (Date->Day - 1);
	}
	DSec = DDay * 86400 + Time ->Hour * 3600 + Time->Min * 60 + Time->Sec;
	return DSec;
}

uint32_t Tamp2UTC(LongInt Sec, Date_UserDataStruct *Date, Time_UserDataStruct *Time, uint32_t LastDDay)
{
	uint32_t DayTable[2][12] = { { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 }, { 0, 31, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 } };
	uint32_t DYear, LDYear,  i, LeapFlag;
	uint32_t DDay;
	DDay = Sec / 86400;

	if (DDay != LastDDay)
	{
		Time->Week = (4 + DDay) % 7;
		LDYear = DDay / 365;
		if (DDay >= (LDYear * 365 + ((LDYear + 2) / 4) - ((LDYear + 2) / 100) + ((LDYear + 2) / 400)))
		{
			DYear = LDYear;
		}
		else
		{
			DYear = LDYear - 1;
		}
		Date->Year = DYear + 1970;
		LeapFlag = IsLeapYear(DYear + 1970);
		if (Date->Year > 1970)
		{
			DDay -= (DYear * 365 + ((DYear + 2) / 4) - ((DYear + 2) / 100) + ((DYear + 2) / 400));
		}

		Date->Mon = 12;
		for (i = 1; i < 12; i++)
		{
			if (DDay < DayTable[LeapFlag][i])
			{
				Date->Mon = i;
				break;
			}
		}
		Date->Day = DDay - DayTable[LeapFlag][Date->Mon - 1] + 1;
	}

	Sec = Sec % 86400;
	Time->Hour = Sec / 3600;
	Sec = Sec % 3600;
	Time->Min = Sec / 60;
	Time->Sec = Sec % 60;
	return DDay;
}


uint8_t XorCheck(void *Src, uint32_t Len, uint8_t CheckStart)
{
	uint8_t Check = CheckStart;
	uint8_t *Data = (uint8_t *)Src;
	uint32_t i;
	for (i = 0; i < Len; i++)
	{
		Check ^= Data[i];
	}
	return Check;
}

/*
 * 转义打包
 * 标识Flag，即包头包尾加入Flag
 * 数据中遇到Flag -> Code F1
 * 数据中遇到Code -> Code F2
 */

uint32_t TransferPack(uint8_t Flag, uint8_t Code, uint8_t F1, uint8_t F2, uint8_t *InBuf, uint32_t Len, uint8_t *OutBuf)
{
	uint32_t TxLen = 0;
	uint32_t i;
	OutBuf[0] = Flag;
	TxLen = 1;
	for (i = 0; i < Len; i++)
	{
		if (InBuf[i] == Flag)
		{
			OutBuf[TxLen++] = Code;
			OutBuf[TxLen++] = F1;
		}
		else if (InBuf[i] == Code)
		{
			OutBuf[TxLen++] = Code;
			OutBuf[TxLen++] = F2;
		}
		else
		{
			OutBuf[TxLen++] = InBuf[i];
		}
	}
	OutBuf[TxLen++] = Flag;
	return TxLen;
}

/*
 * 转义解包
 * 标识Flag，即包头包尾加入Flag
 * 数据中遇到Code F1 -> Flag
 * 数据中遇到Code F2 -> Code
 * 数据中遇到Flag 出错返回0
 */

uint32_t TransferUnpack(uint8_t Flag, uint8_t Code, uint8_t F1, uint8_t F2, uint8_t *InBuf, uint32_t Len, uint8_t *OutBuf)
{
	uint32_t RxLen = 0;
	uint32_t i = 0;
	while (i < Len)
	{
		if (InBuf[i] == Code)
		{
			if (InBuf[i+1] == F1)
			{
				OutBuf[RxLen++] = Flag;
			}
			else if (InBuf[i+1] == F2)
			{
				OutBuf[RxLen++] = Code;
			}
			else
			{
				return 0;
			}
			i += 2;
		}
		else if (InBuf[i] == Flag)
		{
			return 0;
		}
		else
		{
			OutBuf[RxLen++] = InBuf[i++];
		}
	}
	return RxLen;
}

//kalman一维滤波算法
double KalmanSingleFilter(KalmanFilter_Struct *Filter, double RealTimeValue)
{
	double XTemp, PTemp, Kg, Xnow, Pnow;
	XTemp = Filter->LastProcessResult_X;
	PTemp = Filter->LastCovariance_P + Filter->ProcessNoise_Q;
	Kg = PTemp / (PTemp + Filter->MeasureNoise_R);
	Xnow = XTemp + Kg * (RealTimeValue - XTemp);
	Pnow = (1 - Kg) * PTemp;
	Filter->LastProcessResult_X = Xnow;
	Filter->LastCovariance_P = Pnow;
	return Xnow;
}

/**
* @brief  反转数据
* @param  ref 需要反转的变量
* @param	ch 反转长度，多少位
* @retval N反转后的数据
*/
static LongInt Reflect(LongInt ref, uint8_t ch)
{
	unsigned long long value = 0;
	uint32_t i;
	for (i = 1; i< (ch + 1); i++)
	{
		if (ref & 1)
			value |= (LongInt)1 << (ch - i);
		ref >>= 1;
	}
	return value;
}

/**
* @brief  建立CRC32的查询表
* @param  Tab 表缓冲
* @param	Gen CRC32根
* @retval None
*/
void CRC32_CreateTable(uint32_t *Tab, uint32_t Gen)
{
	uint32_t crc;
	uint32_t i, j, temp, t1, t2, flag;
	if (Tab[1] != 0)
		return;
	for (i = 0; i < 256; i++)
	{
		temp = Reflect(i, 8);
		Tab[i] = temp << 24;
		for (j = 0; j < 8; j++)
		{
			flag = Tab[i] & 0x80000000;
			t1 = Tab[i] << 1;
			if (0 == flag)
			{
				t2 = 0;
			}
			else
			{
				t2 = Gen;
			}
			Tab[i] = t1 ^ t2;
		}
		crc = Tab[i];
		Tab[i] = Reflect(crc, 32);
	}
}


/**
* @brief  计算buffer的crc校验码
* @param  CRC32_Table CRC32表
* @param  Buf 缓冲
* @param	Size 缓冲区长度
* @param	CRC32 初始CRC32值
* @retval 计算后的CRC32
*/
uint32_t CRC32_Cal(uint32_t *CRC32_Table, uint8_t *Buf, uint32_t Size, uint32_t CRC32Last)
{
	uint32_t i;
	for (i = 0; i < Size; i++)
	{
		CRC32Last = CRC32_Table[(CRC32Last ^ Buf[i]) & 0xff] ^ (CRC32Last >> 8);
	}
	return CRC32Last;
}

/************************************************************************/
/*  CRC16                                                                */
/************************************************************************/
uint16_t CRC16Cal(uint8_t *Src, uint16_t Len, uint16_t CRC16Last, uint16_t CRCRoot, uint8_t IsReverse)
{
	uint16_t i;
	uint16_t CRC16 = CRC16Last;
	uint16_t wTemp = CRCRoot;


	if (IsReverse)
	{
		CRCRoot = 0;
		for (i = 0; i < 16; i++)
		{
			if (wTemp & (1 << (15 - i)))
			{
				CRCRoot |= 1 << i;
			}
		}
		while (Len--)
		{
			for (i = 0; i < 8; i++)
			{
				if ((CRC16 & 0x0001) != 0)
				{
					CRC16 >>= 1;
					CRC16 ^= CRCRoot;
				}
				else
				{
					CRC16 >>= 1;
				}
				if ((*Src&(1 << i)) != 0)
				{
					CRC16 ^= CRCRoot;
				}
			}
			Src++;
		}
	}
	else
	{
		while (Len--)
		{
			for (i = 8; i > 0; i--)
			{
				if ((CRC16 & 0x8000) != 0)
				{
					CRC16 <<= 1;
					CRC16 ^= CRCRoot;
				}
				else
				{
					CRC16 <<= 1;
				}
				if ((*Src&(1 << (i - 1))) != 0)
				{
					CRC16 ^= CRCRoot;
				}
			}
			Src++;
		}
	}

	return CRC16;
}





void InitRBuffer(RBuffer *Buf, void *Src, uint32_t MaxLen, uint32_t DataSize)
{
	uint8_t *Data = (uint8_t *)Src;
	Buf->Data = Data;
	Buf->Len = 0;
	Buf->MaxLength = MaxLen;
	Buf->Offset = 0;
	Buf->DataSize = DataSize;
}

uint32_t QueryRBuffer(RBuffer *Buf, void *Src, uint32_t Len)
{
	uint32_t i, p;
	uint8_t *Data = (uint8_t *)Src;
	if (Buf->Len < Len)
	{
		Len = Buf->Len;
	}
	if (Buf->DataSize > 1)
	{
		for (i = 0, p = Buf->Offset; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}
			memcpy(Data + (i * Buf->DataSize), Buf->Data + (p * Buf->DataSize), Buf->DataSize);
		}
	} 
	else
	{
		for (i = 0, p = Buf->Offset; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}
			Data[i] = Buf->Data[p];
		}
	}



	//for query, don't update r_buffer control struct.
	//buf->len -= len;
	//buf->offset = p;

	return Len;
}

uint32_t ReadRBuffer(RBuffer *Buf, void *Src, uint32_t Len)
{
	uint32_t l;
	uint8_t *Data = (uint8_t *)Src;
	l = QueryRBuffer(Buf, Data, Len);
	Buf->Len -= l;
	Buf->Offset += l;
	if (Buf->Offset > Buf->MaxLength)
	{
		Buf->Offset -= Buf->MaxLength;

	}
	return l;
}

void DelRBuffer(RBuffer *Buf, uint32_t Len)
{
	if (Buf->Len < Len)
	{
		Len = Buf->Len;
	}

	Buf->Len -= Len;
	Buf->Offset += Len;
	if (Buf->Offset > Buf->MaxLength)
	{
		Buf->Offset -= Buf->MaxLength;
	}
		
	if (Buf->Len == 0) {
		Buf->Offset = 0;
	}
}

uint32_t WriteRBufferForce(RBuffer *Buf, void *Src, uint32_t Len)
{
	uint32_t i, p, cut_off = 0;
	uint8_t *Data = (uint8_t *)Src;
	cut_off = Buf->MaxLength - Buf->Len;
	if (cut_off >= Len)
	{
		cut_off = 0;
	}		
	else
	{
		cut_off = Len - cut_off;
	}

	if (Buf->DataSize > 1)
	{
		for (i = 0, p = Buf->Offset + Buf->Len; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}
			memcpy(Buf->Data + (p * Buf->DataSize), Data + (i * Buf->DataSize), Buf->DataSize);
		}
	} 
	else
	{
		for (i = 0, p = Buf->Offset + Buf->Len; i < Len; i++, p++)
		{
			if (p >= Buf->MaxLength)
			{
				p -= Buf->MaxLength;
			}

			Buf->Data[p] = Data[i];
		}
	}


	Buf->Offset += cut_off;
	if (Buf->Offset >= Buf->MaxLength)
		Buf->Offset -= Buf->MaxLength;

	Buf->Len += Len;
	if (Buf->Len > Buf->MaxLength)
		Buf->Len = Buf->MaxLength;

	return Len;
		
}




