#include <LQ_GPIO.h>
#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>

/*************************************************************************
*  函数名称：void Test_GPIO_Extern_Int(void)
*  功能说明：测试外部中断
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  备    注：核心板上的LED固定时间P10.6翻转，LED灯闪烁，
*            当P15.4检测到下降沿时触发中断，P10.5翻转，LED闪烁，中断函数在LQ_GPIO.C中
*            可以把P15.4用杜邦线接到P10.6，触发中断后亮灯依次闪烁
*************************************************************************/
void Test_GPIO_Extern_Int(void)
{
	PIN_Exti(P15_4, PIN_IRQ_MODE_FALLING);//配置P15_4 下降沿触发中断
	while(1)
	{
    	LED_Ctrl(LED0,RVS);        //电平翻转,LED闪烁
		delayms(500);              //延时等待
	}
}

