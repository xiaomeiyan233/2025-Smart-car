#include <LQ_GPIO.h>
#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>

BuzzerStruct buzzerStr;
//======延时数组======//
unsigned int Delay_100ms[10] = {0};
unsigned int Count[10] = {0};
//=================//
//BuzzerInit
void BuzzerInit(void)
{
    PIN_InitConfig(BUZZER_PIN, PIN_MODE_OUTPUT, 1);
    buzzerStr.Counter = 0;
    buzzerStr.Cut = 0;
    buzzerStr.Enable = 0;
    buzzerStr.Times = 0;
}
/**
* @brief        GPIO线程控制器
**/
void GPIO_Timer(void)
{
    //蜂鸣器控制
    if(buzzerStr.Enable)
    {
        buzzerStr.Counter++;

        if(buzzerStr.Cut<buzzerStr.Counter)
            buzzerStr.Counter = buzzerStr.Cut;
    }
}
/**
* @brief        GPIO逻辑处理函数
**/
void GPIO_Handle(void)
{
     //蜂鸣器控制
     if(buzzerStr.Enable && !buzzerStr.Silent)
     {
         if(buzzerStr.Times<=0)
         {
             BUZZER_OFF;
             buzzerStr.Enable = 0;
             LED_Ctrl(LED2, OFF);
             return;
         }
         else if(buzzerStr.Cut<=buzzerStr.Counter)
         {
             BUZZER_REV;
             buzzerStr.Times--;
             buzzerStr.Counter = 0;
         }
     }
     else
         BUZZER_OFF;

    //LED闪烁伴随
     if(Delay_100ms[3] == 1 && buzzerStr.Times > 0)
     {
         Delay_100ms[3] = 0;
         LED_Ctrl(LED2, RVS);
     }
}
/**
* @brief        蜂鸣器使能
* @param        buzzer：蜂鸣器工作模式
**/
void GPIO_BuzzerEnable(BuzzerEnum buzzer)
{
 switch(buzzer)
 {
     case BuzzerOk:
         buzzerStr.Cut = 80;         //80ms
         buzzerStr.Enable = 1;
         buzzerStr.Times = 3;
         break;

     case BuzzerWarnning:
         buzzerStr.Cut = 100;        //100ms
         buzzerStr.Enable = 1;
         buzzerStr.Times = 12;
         break;

     case BuzzerSysStart:
         buzzerStr.Cut = 200;         //200ms
         buzzerStr.Enable = 1;
         buzzerStr.Times = 6;
         break;

     case BuzzerDing:
         buzzerStr.Cut = 40;         //40ms
         buzzerStr.Enable = 1;
         buzzerStr.Times = 2;
         break;

     case BuzzerFinish:
         buzzerStr.Cut = 90;        //90ms
         buzzerStr.Enable = 1;
         buzzerStr.Times = 6;
         break;
 }
 buzzerStr.Counter = 0;
}

/*************************************************************************
*  函数名称：void LED_Init(void)
*  功能说明：GPIO初始化函数 LED灯所用P10.6、P10.5、P15.4和P15.6初始化
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  备    注：
*************************************************************************/
void GPIO_LED_Init(void)
{
	  // 初始化,输入口，高电平
	  PIN_InitConfig(LED0p, PIN_MODE_OUTPUT, 0);
	  PIN_InitConfig(LED1p, PIN_MODE_OUTPUT, 0);
	  PIN_InitConfig(LED2p, PIN_MODE_OUTPUT, 0);
	  PIN_InitConfig(LED3p, PIN_MODE_OUTPUT, 0);
}

/*************************************************************************
*  函数名称：void LED_Ctrl(LEDn_e LEDno, LEDs_e sta)
*  功能说明：LED控制
*  参数说明：LEDn_e LEDno编号,LEDs_e sta状态亮/灭/翻转
*  函数返回：按键状态，0/1
*************************************************************************/
void LED_Ctrl(LEDn_e LEDno, LEDs_e sta)
{
    switch(LEDno)
    {
    case LED0:
      if(sta==ON)        PIN_Write(LED0p,0);
      else if(sta==OFF) PIN_Write(LED0p,1);
      else if(sta==RVS) PIN_Reverse(LED0p);
    break;

    case LED1:
      if(sta==ON)        PIN_Write(LED1p,0);
      else if(sta==OFF) PIN_Write(LED1p,1);
      else if(sta==RVS) PIN_Reverse(LED1p);
    break;

    case LED2:
      if(sta==ON)        PIN_Write(LED2p,0);
      else if(sta==OFF) PIN_Write(LED2p,1);
      else if(sta==RVS) PIN_Reverse(LED2p);
    break;

    case LED3:
      if(sta==ON)        PIN_Write(LED3p,0);
      else if(sta==OFF) PIN_Write(LED3p,1);
      else if(sta==RVS) PIN_Reverse(LED3p);
    break;
    case LEDALL:
      if(sta==ON)
      {
    	  PIN_Write(LED0p,0);
    	  PIN_Write(LED1p,0);
    	  PIN_Write(LED2p,0);
    	  PIN_Write(LED3p,0);
      }
      else if(sta==OFF)
      {
    	  PIN_Write(LED0p,1);
    	  PIN_Write(LED1p,1);
    	  PIN_Write(LED2p,1);
    	  PIN_Write(LED3p,1);
      }
      else if(sta==RVS)
      {
    	  PIN_Reverse(LED0p);
    	  PIN_Reverse(LED1p);
    	  PIN_Reverse(LED2p);
    	  PIN_Reverse(LED3p);
      }
    break;
    default:
    break;
    }
}
/*************************************************************************
*  函数名称：void Test_GPIO_LED(void)
*  功能说明：测试程序
*  参数说明：无
*  函数返回：无
*  修改时间：2020年3月10日
*  备    注：核心板上的LED灯闪烁--四个LED同时闪烁
*************************************************************************/
void Test_GPIO_LED(void)
 {
	GPIO_LED_Init();
 	while(1)
 	{
 		LED_Ctrl(LEDALL,RVS);        //四个LED同时闪烁
 		delayms(100);                //延时等待
 	}
 }
