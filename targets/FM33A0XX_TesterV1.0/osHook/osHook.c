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
__IO static UINT32 sleepTicks;
static UINT32 osTicks2TimerCount( UINT32 ticks )
{
    return ( UINT32 )( ticks<<10 )/LOSCFG_BASE_CORE_TICK_PER_SECOND;
}

static UINT32 timerCount2OsTicks( UINT32 count )
{
    UINT32 seconds = count >> 10;

    count = count & 0x3FF;
    return ( ( seconds * LOSCFG_BASE_CORE_TICK_PER_SECOND ) + ( ( count * LOSCFG_BASE_CORE_TICK_PER_SECOND ) >> 10 ) );
}
static void LPTIM_IRQHandler()
{
  IWDT_Clr();	
  if(LPTIM_LPTIF_OVIF_Chk()==SET)
  {
    LPTIM_LPTIF_OVIF_Clr();
    sleepTicks = ((1<<16)-1);
  }
  else if(LPTIM_LPTIF_COMPIF_Chk()==SET)//比较事件
  {
    LPTIM_LPTIF_COMPIF_Clr();
    LPTIM_LPTIE_COMPIE_Setable(DISABLE);
    sleepTicks = LPTIM_LPTCMP_Read();
    return ;
  }
}

static void lptimerInit(void)
{
	LPTIM_InitTypeDef init_para;

	//使能LPTIMER的外设时钟
	RCC_PERCLK_SetableEx( LPTFCLK, 		DISABLE );		//LPTIM功能时钟使能
	RCC_PERCLK_SetableEx( LPTRCLK, 		DISABLE );		//LPTIM总线时钟使能
	RCC_PERCLK_SetableEx( LPTFCLK, 		ENABLE );		//LPTIM功能时钟使能
	RCC_PERCLK_SetableEx( LPTRCLK, 		ENABLE );		//LPTIM总线时钟使能
	LPTIM_Deinit();

	init_para.LPTIM_TMODE = LPTIM_LPTCFG_TMODE_PWMIM;//设置工作模式为TIMEOUT
	init_para.LPTIM_MODE = LPTIM_LPTCFG_MODE_CONTINUE;//设置计数模式为连续计数模式

	init_para.LPTIM_PWM = LPTIM_LPTCFG_PWM_TOGGLE;//选择PWM输出模式
	init_para.LPTIM_POLAR = LPTIM_LPTCFG_POLARITY_POS;//产生输出波形上升沿
  init_para.LPTIM_target_value = 0xFFFF;

	init_para.LPTIM_TRIG_CFG = LPTIM_LPTCFG_TRIGCFG_POS;//外部输入信号上升沿trigger
	init_para.LPTIM_FLTEN = DISABLE;//使能数字滤波
	
	init_para.LPTIM_LPTIN_EDGE = LPTIM_LPTCFG_EDGESEL_POS;
	
	init_para.LPTIM_CLK_SEL = LPTIM_LPTCFG_CLKSEL_LSCLK;//选择外设时钟
	init_para.LPTIM_CLK_DIV = LPTIM_LPTCFG_DIVSEL_32;//32分频，如果PCLK为8M,则分频时钟为250k

	LPTIM_Init(&init_para);//初始化LPTIMER
  LPTIM_LPTCTRL_LPTEN_Setable(ENABLE);
  LPTIM_LPTIE_OVIE_Setable(ENABLE);
  if(sleepTicks<0xFFFF)
  {
    LPTIM_LPTCMP_Write(sleepTicks&0xFFFF);
    LPTIM_LPTIE_COMPIE_Setable(ENABLE);
  }
  osSetVector(LPTIM_IRQn,LPTIM_IRQHandler);
	//使能LPTIMER的外设中断
	NVIC_DisableIRQ(LPTIM_IRQn);
	NVIC_SetPriority(LPTIM_IRQn, 2);
	NVIC_EnableIRQ(LPTIM_IRQn);		
}


void wakeup_config(void)
{
  lptimerInit();//初始化LPTIMER
}

void sleepConfig(void)
{
	PMU_SleepCfg_InitTypeDef SleepCfg_InitStruct;
	
	ANAC_PDRCON_PDREN_Setable(ENABLE);	
	ANAC_BORCON_OFF_BOR_Setable(ENABLE);	
	
	RCC_SYSCLKSEL_LPM_RCLP_OFF_Setable(DISABLE);	
	
	SleepCfg_InitStruct.PMOD = PMU_LPMCFG_PMOD_SLEEP;			
	SleepCfg_InitStruct.SLPDP = PMU_LPMCFG_SLPDP_DEEPSLEEP;			
	SleepCfg_InitStruct.DSLPRAM_EXT = PMU_LPMCFG_DSLPRAM_EXT_ALLRAMON;
	SleepCfg_InitStruct.CVS = DISABLE;							
	SleepCfg_InitStruct.XTOFF = PMU_LPMCFG_XTOFF_DIS;			
	SleepCfg_InitStruct.SCR = 0;								
	
	PMU_SleepCfg_Init(&SleepCfg_InitStruct);
	
	IWDT_Clr();	
		
}
void userEnterSleepHook(void)
{ 
  if(sleepTicks>0)
  {
    wakeup_config();
    sleepConfig();
  }
  return;
}
void userExitSleepHook(void)
{
  
  LPTIM_Deinit();
  return;
}
UINT32 getSleepTicks(void)
{
  return timerCount2OsTicks(sleepTicks);
}
void setSleepTicks(UINT32 ticks)
{
  sleepTicks = osTicks2TimerCount(ticks);  
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif  /* end of #if (LOSCFG_KERNEL_TICKLESS == YES) */

