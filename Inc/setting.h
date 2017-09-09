#ifndef __SETTING_H__
#define __SETTING_H__
/* Private typedef -----------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Private define ------------------------------------------------------------*/


/*******************以下内容只与被测主板有关，于测试板本身无关***********************************/
//以下定义了被观测到的动态变量
enum {
	CPUID1,
	CPUID2,
	CPUID3,
	FLASHSIZE,
	SOFTVERSION,
	SYSCLK_FRQ, /*!<  SYSCLK clock frequency expressed in Hz */
	HCLK_FRQ,   /*!<  HCLK clock frequency expressed in Hz   */
	PCLK1_FRQ,  /*!<  PCLK1 clock frequency expressed in Hz  */
	PCLK2_FRQ,  /*!<  PCLK2 clock frequency expressed in Hz  */
	SYS_MODE,//包括了调试模式，运行模式
	CURRENT_DATE,//当前日期
	CURRENT_TIME,//当前时间
	POWER_AD0,
	AD_MOTOR_B = POWER_AD0,
	AD_MOTOR_M,
	AD_MOTOR_D,
	AD_WHEEL_L,
	AD_WHEEL_R,
	AD_BAT_VOL,
	AD_BAT_CUR,
	AD_BAT_TEMP,
	IR_NOLIGHT_AD0,
	AD_IR_FL_NL = IR_NOLIGHT_AD0,
	AD_IR_FML_NL,
	AD_IR_FMR_NL,
	AD_IR_FR_NL,
	AD_IR_DL_NL,
	AD_IR_DM_NL,
	AD_IR_DR_NL,
	AD_IR_BL_NL,
	IR_LIGHT_AD0,
	AD_IR_FL = IR_LIGHT_AD0,
	AD_IR_FML,
	AD_IR_FMR,
	AD_IR_FR,
	AD_IR_DL,
	AD_IR_DM,
	AD_IR_DR,
	AD_IR_BL,
	CURRENT_KEY,
	MOTOR_ARR,
	WHEEL_ARR,
	MOTOR_BIANSAO_DUTY,
	MOTOR_MID_DUTY,
	MOTOR_DUST_DUTY,
	WHEEL_LEFT_DUTY,
	WHEEL_RIGHT_DUTY,
	WHEEL_LEFT_TARGET_INT,//左轮期望中断
	WHEEL_RIGHT_TARGET_INT,//右轮期望中断
	WHEEL_LEFT_SPEED_INT,//实际左轮中断
	WHEEL_RIGHT_SPEED_INT,//实际右轮中断
	WHEEL_MID_SPEED_INT,//实际万向轮中断
	MOTOR_WHEEL_CMD,//车轮电机控制命令
	DOCK_SIGNAL1,
	DOCK_SIGNAL2,
	WIFI_STATE,
	ERROR_CODE,
	HEART,
	VAR_MAX,

};

//以下定义了被观测到的硬件状态
enum
{
	HSE_DETECTED = 0,
	LSE_DETECTED,
	NRF24L01_DETECTED,//NRF24L01接收模块
	NRF24L01_2_DETECTED,//NRF24L01发送模块
	RTC_RUN,
	WIFI_DETECTED,
	RUB_BOX_DETECTED,//垃圾箱检测到
	MOTOR_BIANSAO_OVERLOAD,
	MOTOR_MID_OVERLOAD,
	MOTOR_DUST_OVERLOAD,
	MOTOR_WHEEL_L_OVERLOAD,
	MOTOR_WHEEL_R_OVERLOAD,
	MOTOR_UNUSE_OVERLOAD,
	MOTOR_WHEEL_LEFT_PICKUP,
	MOTOR_WHEEL_RIGHT_PICKUP,
	WHEEL_CHIP_DOWN,
	MOTOR_CHIP_DOWN,
	CHARGE_DETECTED,//检测到充电状态
	LUKNOCK_DETECTED,
	LDKNOCK_DETECTED,
	RUKNOCK_DETECTED,
	RDKNOCK_DETECTED,
	DEVICE_STATE_MAX,
};

//以下定义了需要立刻停机的致命故障
enum
{
	ERR_NO_RUB_BOX,

	ERR_M_SPEED_DETECT,//万向轮测速故障
	ERR_L_SPEED_DETECT,//左轮测速故障
	ERR_R_SPEED_DETECT,//右轮测速故障

	MOTOR_CUR0_OVERLOAD,
	ERR_MOTOR_BS_OVERLOAD = MOTOR_CUR0_OVERLOAD,
	ERR_MOTOR_MID_OVERLOAD,
	ERR_MOTOR_DUST_OVERLOAD,
	ERR_WHEEL_L_OVERLOAD,
	ERR_WHEEL_R_OVERLOAD,
	ERR_WHEEL_PICKUP,
	ERR_WHEEL_OCD_DOWN,

	ERR_MOTOR_BS_NOCUR,
	ERR_MOTOR_MID_NOCUR,
	ERR_MOTOR_DUST_NOCUR,
	ERR_WHEEL_L_NOCUR,
	ERR_WHEEL_R_NOCUR,
};
/*******************以上内容只与被测主板有关，于测试板本身无关***********************************/

void System_VarInit(void);
void Config_Init(void);
void Config_Save(void);
unsigned int CRC32_Calculate(unsigned char *Buf, unsigned int Size, unsigned int CRC32);
void Upgrade(unsigned int FileLen);
#endif
