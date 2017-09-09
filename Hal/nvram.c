#include "user.h"

int NVRAM_Write(uint32_t Address, void *Data, unsigned int Size)
{
	u32 Check;
	u32 i;
	u32 *iData;
	volatile FLASH_Status FLASHStatus;
	FLASHStatus = FLASH_COMPLETE;
	if (Size > FLASH_PAGE_SIZE)
	{
		Size = FLASH_PAGE_SIZE;
	}
	NVRAM_Erase(Address);
	FLASH_Unlock();
	//	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR |
	//	                  FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR|FLASH_FLAG_PGSERR);
		//	FLASH_EraseSector(FLASH_Sector_5, VoltageRange_3);
	FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
	i = 0;
	iData = (uint32_t *)Data;
	while( ( i < (Size/4) ) && (FLASHStatus == FLASH_COMPLETE) )
	{

		FLASHStatus = FLASH_ProgramWord(Address, iData[i]);
		memcpy(&Check, (uint32_t *)Address, 4);
		if (memcmp(&Check, &iData[i], 4))
		{
			DBG_ERR("Flash write error in %x %x %x",Address, Check, iData[i]);
		}
		Address = Address + 4;
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
