#include <LQ_GPIO_LED.h>
#include <LQ_CCU6_Timer.h>
#include <LQ_CCU6.h>

 void LQ_CCU6_Timer (void)
  {
     CCU6_InitConfig(CCU60, CCU6_Channel0, 200000);//CCU6初始化
     CCU6_InitConfig(CCU61, CCU6_Channel0, 400000);//CCU6初始化
     CCU6_InitConfig(CCU60, CCU6_Channel1, 500000);//CCU6初始化
     CCU6_InitConfig(CCU61, CCU6_Channel1, 1000000);//CCU6初始化
#pragma warning 557         // 屏蔽警告
	//中断服务函数中翻转LED
  	while(1)
  	{

  	}
  }
