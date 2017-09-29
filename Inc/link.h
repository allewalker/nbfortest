#ifndef __LINK_H__
#define __LINK_H__
void Link_Init(void);
void Link_UrcAnalyze(int8_t *Str);
void Link_RxData(uint8_t *Data, uint16_t Len);
void Link_TxData(void);
#endif
