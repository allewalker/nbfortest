#include "stm32f10x.h"
#include "user.h"
#define SYSCLK_FREQ_72MHz  72000000

#if (__CUST_TYPE__ == __CUST_BOOTLOADER__ || !defined(__SYS_START_FROM_BOOTLOADER__))
#define VECT_TAB_OFFSET  0x0 /*!< Vector Table base offset field.
                                  This value must be a multiple of 0x200. */
#define __RCC_INIT__
#else

#define VECT_TAB_OFFSET  APP_FLASH_START /*!< Vector Table base offset field.
                                  This value must be a multiple of 0x200. */

#endif //__CUST_TYPE__

//__I uint8_t AHBPrescTable[16] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 3, 4, 6, 7, 8, 9};

void SystemInit (void)
{
#ifdef __RCC_INIT__
	__IO uint32_t StartUpCounter = 0, HSEStatus = 0, LSEStatus = 0, RTCClock = 0;
	  /* Reset the RCC clock configuration to the default reset state(for debug purpose) */
	  /* Set HSION bit */
	RCC->CR |= (uint32_t)0x00000001;

	  /* Reset SW, HPRE, PPRE1, PPRE2, ADCPRE and MCO bits */
#ifndef STM32F10X_CL
	RCC->CFGR &= (uint32_t)0xF8FF0000;
#else
	RCC->CFGR &= (uint32_t)0xF0FF0000;
#endif /* STM32F10X_CL */

	  /* Reset HSEON, CSSON and PLLON bits */
	RCC->CR &= (uint32_t)0xFEF6FFFF;

	  /* Reset HSEBYP bit */
	RCC->CR &= (uint32_t)0xFFFBFFFF;

	  /* Reset PLLSRC, PLLXTPRE, PLLMUL and USBPRE/OTGFSPRE bits */
	RCC->CFGR &= (uint32_t)0xFF80FFFF;

#ifdef STM32F10X_CL
	  /* Reset PLL2ON and PLL3ON bits */
	RCC->CR &= (uint32_t)0xEBFFFFFF;

	  /* Disable all interrupts and clear pending bits  */
	RCC->CIR = 0x00FF0000;

	  /* Reset CFGR2 register */
	RCC->CFGR2 = 0x00000000;
#elif defined (STM32F10X_LD_VL) || defined (STM32F10X_MD_VL) || (defined STM32F10X_HD_VL)
	  /* Disable all interrupts and clear pending bits  */
	RCC->CIR = 0x009F0000;

	  /* Reset CFGR2 register */
	RCC->CFGR2 = 0x00000000;
#else
	  /* Disable all interrupts and clear pending bits  */
	RCC->CIR = 0x009F0000;
#endif /* STM32F10X_CL */

#if defined (STM32F10X_HD) || (defined STM32F10X_XL) || (defined STM32F10X_HD_VL)
#ifdef DATA_IN_ExtSRAM
	SystemInit_ExtMemCtl();
#endif /* DATA_IN_ExtSRAM */
#endif


	/* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/
	/* Enable HSE */
	RCC->CR |= ((uint32_t)RCC_CR_HSEON);

	if (!(RCC->BDCR & RCC_BDCR_LSERDY))//LSE没有启动
	{
		RCC->APB1ENR |= RCC_APB1ENR_PWREN|RCC_APB1ENR_BKPEN;
		PWR->CR |= PWR_CR_DBP;//允许修改BACK区
		RCC->BDCR |= ((uint32_t)RCC_BDCR_BDRST);
		RCC->BDCR &= ~((uint32_t)RCC_BDCR_BDRST);
		RCC->BDCR = RCC_BDCR_RTCEN|RCC_BDCR_RTCSEL_0|RCC_BDCR_LSEON;
	}
	/* Wait till HSE is ready and if Time out is reached exit */
	do
	{
		HSEStatus = RCC->CR & RCC_CR_HSERDY;
		StartUpCounter++;
	} while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

	if ((RCC->CR & RCC_CR_HSERDY) != RESET)
	{
		HSEStatus = (uint32_t)0x01;
	}
	else
	{
		HSEStatus = (uint32_t)0x00;
	}

//	StartUpCounter = 0;
//	do
//	{
//		LSEStatus = RCC->BDCR & RCC_BDCR_LSERDY;
//		StartUpCounter++;
//	}while((LSEStatus == 0) && (StartUpCounter != 0x0006FFFF));
//	LSEStatus = RCC->BDCR & RCC_BDCR_LSERDY;

//	if (LSEStatus)
//	{
//		RCC->BDCR = RCC_BDCR_RTCEN|RCC_BDCR_RTCSEL_0|RCC_BDCR_LSEON;
//	}
//	else
//	{
//		//RCC->BDCR = 0;
//	}

	if (HSEStatus == (uint32_t)0x01)
	{
	/* Enable Prefetch Buffer */
		FLASH->ACR |= FLASH_ACR_PRFTBE;

		/* Flash 2 wait state */
		FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
		FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;

		RCC->CFGR |= (uint32_t)RCC_CFGR_ADCPRE_DIV6;
#ifdef STM32F10X_CL
		/* Configure PLLs ------------------------------------------------------*/
		/* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
		/* PREDIV1 configuration: PREDIV1CLK = PLL2 / 5 = 8 MHz */

		RCC->CFGR2 &= (uint32_t)~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
								  RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);
		RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 |
								 RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);

		/* Enable PLL2 */
		RCC->CR |= RCC_CR_PLL2ON;
		/* Wait till PLL2 is ready */
		while((RCC->CR & RCC_CR_PLL2RDY) == 0)
		{
		}


		/* PLL configuration: PLLCLK = PREDIV1 * 9 = 72 MHz */
		RCC->CFGR &= (uint32_t)~(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
		RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLSRC_PREDIV1 |
								RCC_CFGR_PLLMULL9);
#else
		/*  PLL configuration: PLLCLK = HSE * 9 = 72 MHz */
		RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
											RCC_CFGR_PLLMULL));
		RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE | RCC_CFGR_PLLMULL9);
#endif /* STM32F10X_CL */


	}
	else
	{ /* If HSE fails to start-up, the application will have wrong clock
		 configuration. User can add here some code to deal with this error */
		FLASH->ACR |= FLASH_ACR_PRFTBE;

		/* Flash 2 wait state */
		FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
		FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_1;

		RCC->CFGR |= (uint32_t)RCC_CFGR_ADCPRE_DIV4;
		/*  PLL configuration: PLLCLK = HSI / 2 * 9 = 36 MHz */
		RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
											RCC_CFGR_PLLMULL));
		RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSI_Div2 | RCC_CFGR_PLLMULL9);
	}

	/* HCLK = SYSCLK */
	RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

	/* PCLK2 = HCLK */
	RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;

	/* PCLK1 = HCLK / 2*/
	RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;

	/* Enable PLL */
	RCC->CR |= RCC_CR_PLLON;

/* Wait till PLL is ready */
	while((RCC->CR & RCC_CR_PLLRDY) == 0)
	{
	}

/* Select PLL as system clock source */
	RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
	RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

/* Wait till PLL is used as system clock source */
	while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08)
	{
	}
#endif
	SCB->VTOR = FLASH_BASE | VECT_TAB_OFFSET; /* Vector Table Relocation in Internal FLASH. */
	/* 使能全部外设时钟 */
	RCC->AHBENR = 0xffffffff;
	RCC->APB1ENR = 0xffffffff & (~RCC_APB1Periph_I2C2); //close i2c2, enable uart3
	RCC->APB2ENR = 0xffffffff;

}

void SystemCoreClockUpdate (void)
{

}
