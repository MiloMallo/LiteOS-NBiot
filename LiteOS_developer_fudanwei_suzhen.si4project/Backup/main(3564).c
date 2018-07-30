#include "define_all.h"  

int main (void)
{	
  UINT32 uwRet = LOS_OK;
	Init_System();				//系统初始化
	Init_PrintUartx();   //printf
	
  uwRet = LOS_KernelInit();
  if (uwRet != LOS_OK)
  {
      return LOS_NOK;
  }
	printf("hw1 \r\n"); //suzhen
  LOS_Inspect_Entry();
	printf("hw2 \r\n"); //suzhen
	
  #if 0  //test
	//pzh - test liteos api examples
	Example_TskCaseEntry();
	Example_MsgQueue();
	Example_Semphore();
	Example_MutexLock();
	Example_GetTick();
	Example_swTimer();
	Example_list();	
	Example_StaticMem();
  Example_Dyn_Mem();
	Example_Event();
  Example_SndRcvEvent();
	Example_TransformTime();

//Example_Interrupt();
    #endif

  LOS_Start();
}
