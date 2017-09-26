#ifndef __SETTING_H__
#define __SETTING_H__
/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/

enum
{
	PARAM_LS_LOAD,
	PARAM_LS_SAVE,
	UPGRADE_TYPE_MAIN_FLASH = 1,
	UPGRADE_TYPE_SPI_FLASH,
};

typedef struct
{
	uint32_t unuse;
}BL_ParamStruct;

typedef union
{
	BL_ParamStruct Data;
	uint8_t Pad[62];
}BL_ParamUnion;

typedef struct
{
	BL_ParamUnion Param;
	uint16_t CRC16;
}BL_ParamStoreStruct;

typedef struct
{
	uint8_t UpgradeReq;
	uint8_t UpgradeType;
	uint32_t UpgradeEntry;
	uint32_t UpgradeSector;
	uint32_t UpgradeCRC32;
}APP_ParamStruct;

typedef union
{
	APP_ParamStruct Data;
	uint8_t Pad[62];
}APP_ParamUnion;

typedef struct
{
	APP_ParamUnion Param;
	uint16_t CRC16;
}APP_ParamStoreStruct;

void System_VarInit(void);
void BL_ParamLS(uint8_t IsSave);
void APP_ParamLS(uint8_t IsSave);
#endif
