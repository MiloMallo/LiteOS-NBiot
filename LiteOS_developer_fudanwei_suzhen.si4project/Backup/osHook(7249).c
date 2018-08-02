/*----------------------------------------------------------------------------
 * Copyright (c) <2013-2015>, <Huawei Technologies Co., Ltd>
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright notice, this list of
 * conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice, this list
 * of conditions and the following disclaimer in the documentation and/or other materials
 * provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific prior written
 * permission.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------
 * Notice of Export Control Law
 * ===============================================
 * Huawei LiteOS may be subject to applicable export control laws and regulations, which might
 * include those applicable to Huawei LiteOS of U.S. and the country in which you are located.
 * Import, export and usage of Huawei LiteOS in any manner by you shall be in compliance with such
 * applicable export control laws and regulations.
 *---------------------------------------------------------------------------*/


#include "los_hwi.h"
#include "los_tick.ph"
#include "los_tickless.h"
#include "los_hw.h"
#include "define_all.h"

#if (LOSCFG_KERNEL_TICKLESS == YES)

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */
void Wakeup_Cfg(void)
{
	RCC_PERCLK_SetableEx( EXTI2CLK, 	ENABLE );		
	RCC_PERCLK_SetableEx( EXTI1CLK, 	ENABLE );		
	RCC_PERCLK_SetableEx( EXTI0CLK, 	ENABLE );		
	RCC_PERCLK_SetableEx( PDCCLK, 		ENABLE );		
	
	InputtIO( GPIOB, GPIO_Pin_0, IN_PULLUP );	
	GPIO_EXTI_Init(GPIOB, GPIO_Pin_0, EXTI_DISABLE);
	
	NVIC_DisableIRQ(GPIO_IRQn);
	NVIC_ClearPendingIRQ(GPIO_IRQn);	
	GPIO_PINWKEN_SetableEx(PINWKEN_PB0, ENABLE);		
}

void Test_Sleep(void)
{
	PMU_SleepCfg_InitTypeDef SleepCfg_InitStruct;
	
	ANAC_PDRCON_PDREN_Setable(ENABLE);	
	ANAC_BORCON_OFF_BOR_Setable(ENABLE);	
	
	RCC_SYSCLKSEL_LPM_RCLP_OFF_Setable(ENABLE);	
	
	SleepCfg_InitStruct.PMOD = PMU_LPMCFG_PMOD_SLEEP;			
	SleepCfg_InitStruct.SLPDP = PMU_LPMCFG_SLPDP_DEEPSLEEP;			
	SleepCfg_InitStruct.DSLPRAM_EXT = PMU_LPMCFG_DSLPRAM_EXT_16KRAMON;
	SleepCfg_InitStruct.CVS = DISABLE;							
	SleepCfg_InitStruct.XTOFF = PMU_LPMCFG_XTOFF_DIS;			
	SleepCfg_InitStruct.SCR = 0;								
	
	PMU_SleepCfg_Init(&SleepCfg_InitStruct);
	
	IWDT_Clr();	
	__WFI();
	IWDT_Clr();	
	
}
void userEnterSleepHook(void)
{
  return;
}


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif  /* end of #if (LOSCFG_KERNEL_TICKLESS == YES) */

