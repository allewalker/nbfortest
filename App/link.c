#include "user.h"
static Link_CtrlStruct gLink;
void Link_Init(void)
{
	gSys.LinkCtrl = &gLink;
	gLink.Stage = LINK_STATE_INIT;
}

void Link_UrcAnalyze(int8_t *Str)
{

}

void Link_RxData(uint8_t *Data, uint16_t Len)
{

}

void Link_TxData(void)
{

}

void Link_Task(void *Param)
{

}
