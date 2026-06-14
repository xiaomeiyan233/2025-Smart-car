#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>

void STM_Timer_Init(void)
{
    STM_InitConfig(STM0, STM_Channel_0, 1000);//STM初始化,1000us进入一次中断
}
 /*************************************************************************
 *  函数名称：void Test_STM(void)
 *  备    注：核心板上的LED灯闪烁，中断时P10.5/P10.6闪灯
 *************************************************************************/
 void LQ_STM_Timer (void)
  {
	 STM_InitConfig(STM0, STM_Channel_0, 500000);//STM初始化
	 STM_InitConfig(STM0, STM_Channel_1, 1000000);//STM初始化

#pragma warning 557         // 屏蔽警告
	//中断服务函数中翻转LED
  	while(1)
  	{

    	//LED_Ctrl(LED0,RVS);        //电平翻转,LED闪烁
//		delayms(500);              //延时等待
  	}
  }
