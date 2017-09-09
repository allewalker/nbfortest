#ifndef __NVRAM_H__
#define __NVRAM_H__

int NVRAM_Write(uint32_t Address, void *Data, unsigned int Size);
void NVRAM_Erase(uint32_t Address);
#endif
