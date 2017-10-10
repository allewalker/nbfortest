#include "user.h"


typedef struct
{
	MyCBFun_t ISR;
	GPIO_TypeDef * Port;
	uint8_t PinNo;
	uint8_t Mode;
	uint8_t Speed;
	uint8_t IsReverse;	//IO值是否需要反转
	uint8_t InitVal;
	uint8_t WorkMode;//0不工作， 1输出， 2输入
}IO_CtrlStruct;

static IO_CtrlStruct IO_List[PIN_MAX];

void IO_ConfigAll(void)
{
	uint8_t i;
	GPIO_InitTypeDef GPIO_InitStructure;
	for(i = 0; i < PIN_MAX; i++)
	{
		IO_List[i].WorkMode = 0;
		IO_List[i].Mode = GPIO_Mode_Out_PP;
		IO_List[i].Speed = GPIO_Speed_50MHz;
	}
	for(i = 0; i < PIN_MAX; i++)
	{
		if (IO_List[i].WorkMode)
		{
			GPIO_InitStructure.GPIO_Mode = IO_List[i].Mode;
			GPIO_InitStructure.GPIO_Speed = IO_List[i].Speed;
			GPIO_InitStructure.GPIO_Pin = 1 << IO_List[i].PinNo;
			GPIO_Init(IO_List[i].Port, &GPIO_InitStructure);
		}
		IO_Write(i,IO_List[i].InitVal);
	}
}


void IO_Write(uint8_t Sn, uint8_t Val)
{
	if (Sn < PIN_MAX)
	{
		DBGF;
		return;
	}
	if (IO_List[Sn].WorkMode == 1)
	{
		if (IO_List[Sn].IsReverse)
		{
			Val = !Val;
		}
		GPIO_WriteBit(IO_List[Sn].Port, 1 << IO_List[Sn].PinNo, (Val > 0)?Bit_SET:Bit_RESET);
	}

}
uint8_t IO_Read(uint8_t Sn)
{
	uint8_t Val = 0;
	if (Sn < PIN_MAX)
	{
		DBGF;
		return 0;
	}
	if (IO_List[Sn].WorkMode == 2)
	{

		Val = GPIO_ReadInputDataBit(IO_List[Sn].Port, 1 << IO_List[Sn].PinNo);
		if (IO_List[Sn].IsReverse)
		{
			Val = !Val;
		}
	}
	return Val;
}
