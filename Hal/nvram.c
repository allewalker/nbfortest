#include "user.h"

int NVRAM_Write(uint32_t Address, void *Data, unsigned int Size)
{
	//uint16_t Check;
	uint32_t i;
	uint16_t *iData;
	volatile FLASH_Status FLASHStatus;
	FLASHStatus = FLASH_COMPLETE;

	FLASH_Unlock();
	//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	//	                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
		//	FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	i = 0;
	iData = (uint16_t *)Data;
	while( ( i < (Size/2) ) && (FLASHStatus == FLASH_COMPLETE) )
	{

		FLASHStatus = FLASH_ProgramHalfWord(Address, iData[i]);
//		memcpy(&Check, (uint16_t *)Address, 2);
//		if (Check != iData[i])
//		{
//			DBG_ERR("Flash write error in %x %x %x",Address, Check, iData[i]);
//		}
		Address = Address + 2;
		i++;
	}
	if (FLASHStatus != FLASH_COMPLETE)
	{
		DBG_ERR("Flash write error in %x %d",Address, FLASHStatus);
	}
	FLASH_Lock();
	return Size;
}

void NVRAM_Erase(uint32_t Address)
{
	volatile FLASH_Status FLASHStatus;
	FLASHStatus = FLASH_COMPLETE;
	FLASH_Unlock();
//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
//	                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
	//	FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	FLASHStatus = FLASH_ErasePage(Address);
	FLASH_Lock();
}
