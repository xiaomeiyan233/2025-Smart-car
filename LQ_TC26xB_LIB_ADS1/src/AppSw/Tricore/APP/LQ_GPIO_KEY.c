#include <LQ_GPIO.h>
#include <LQ_GPIO_KEY.h>
#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>
#include "LQ_Tom_Servo.h"
#include "LQ_Atom_Motor.h"
#include "LQ_UART_Bluetooth.h"

unsigned char KeyPress;
unsigned char Key_Number = 0;
unsigned short  Key_Time;
unsigned int Set_Servo = 1740;
//舵机调节最大角度例程
/*
void Servo_Test_Form(void)
{
    if(Key_Number == 1)
    {
        Key_Number = 0;
        Set_Servo += 1;
        if(Set_Servo < SERVO_PWM_MAX_L)
            Set_Servo = SERVO_PWM_MAX_L;
        else if(Set_Servo > SERVO_PWM_MAX_R)
            Set_Servo = SERVO_PWM_MAX_R;
        SERVO_SetPwmValue(Set_Servo);
        icarStr.ServoPwmSet = Set_Servo;
    }
    else if(Key_Number == 2)
    {
        Key_Number = 0;
        Set_Servo -= 1;
        if(Set_Servo < SERVO_PWM_MAX_L)
            Set_Servo = SERVO_PWM_MAX_L;
        else if(Set_Servo > SERVO_PWM_MAX_R)
            Set_Servo = SERVO_PWM_MAX_R;
        SERVO_SetPwmValue(Set_Servo);
        icarStr.ServoPwmSet = Set_Servo;
    }
}
*/
/********
 * 通过定时器读取按键按下的时间
 * 按键为Key0
 * **********/
void Timer_Key_Read(void)
{
    static unsigned int Record = 0;
    if( KEY_Read(KEY0) == 0)
    {
        Record++;
    }
    else
    {
        Record = 0;
    }
    if(Record == 60)
    {
        Key_Time = 1001;
        KeyPress = 1;
//       GPIO_BuzzerEnable(BuzzerDing); //按下时间大于60ms
    }
}
//IO口默认为高，按键按下接地
unsigned char Key_Scan(void)
{
    unsigned char Key_Value = 0;
    static unsigned char Old_Data, New_Data;
    if(PIN_Read(KEY1p) == 0)
    {
        Key_Value = 1;
    }
    else if(PIN_Read(KEY2p) == 0)
    {
        Key_Value = 2;
    }
    else if(PIN_Read(KEY0p) == 0)
    {
        Key_Value = 3;
    }
    else
    {
        Key_Value = 0;
    }
    Old_Data = New_Data;
    New_Data = Key_Value;
    if(Old_Data == 1 && New_Data == 0)
    {
        Key_Value = 1;
    }
    else if(Old_Data == 2 && New_Data == 0)
    {
        Key_Value = 2;
    }
    else if(Old_Data == 3 && New_Data == 0)
    {
        Key_Value = 3;
    }
    else
    {
        Key_Value = 0;
    }
    return Key_Value;
}
/*************************************************************************
*  函数名称：void GPIO_KEY_Init(void)
*  功能说明：GPIO初始化函数
*************************************************************************/
void GPIO_KEY_Init(void)
{
	  // 初始化,输入口，高电平
	  PIN_InitConfig(KEY0p, PIN_MODE_INPUT, 1);
	  PIN_InitConfig(KEY1p, PIN_MODE_INPUT, 1);
	  PIN_InitConfig(KEY2p, PIN_MODE_INPUT, 1);
	  PIN_InitConfig(DSW0p, PIN_MODE_INPUT, 1);
	  PIN_InitConfig(DSW1p, PIN_MODE_INPUT, 1);
}
/*************************************************************************
*  函数名称：void Reed_Init(void)
*  功能说明：干簧管GPIO及中断初始化函数
*  备    注 中断在那个核初始化，中断VECTABNUM号必须对应该cpu,否则不会调用中断函数
*          中断服务函数PIN_INT0_IRQHandler在LQ_GPIO.c中
*************************************************************************/
void Reed_Init(void)
{
      // 初始化,输入口，高电平
      PIN_InitConfig(REEDp, PIN_MODE_INPUT, 1);
      // 使能场中断 ，下降沿触发
      PIN_Exti(REEDp, PIN_IRQ_MODE_FALLING);
}
#pragma warning 544         // 屏蔽警告

/*************************************************************************
*  函数名称：unsigned char KEY_Read(KEYn_e KEYno)
*  功能说明：读取按键状态
*  参数说明：KEYn_e KEYno按键编号
*  函数返回：按键状态，0/1
*  修改时间：2020年3月10日
*  备    注：
*************************************************************************/
unsigned char KEY_Read(KEYn_e KEYno)
{
    switch(KEYno)
    {
      case KEY0:
        return PIN_Read(KEY0p);//母板上按键0
      break;

      case KEY1:
        return PIN_Read(KEY1p);//母板上按键1
      break;

      case KEY2:
        return PIN_Read(KEY2p);//母板上按键2
      break;

      case DSW0:
        return PIN_Read(DSW0p);//母板上拨码开关0
      break;

      case DSW1:
        return PIN_Read(DSW1p);//母板上拨码开关1
      break;
      default:
        return 0XFF;
    }
    return 0;
}
#pragma warning default     // 打开警告


/*************************************************************************
*  函数名称：unsigned char KEY_Read_All(void)
*  功能说明：读取全部按键状态
*  参数说明：无
*  函数返回：按键组合状态，0--7八种状态
*  修改时间：2020年3月10日
*  备    注：读取三个按键状态，方便组合键使用
*************************************************************************/
unsigned char KEY_Read_All(void)
{
   unsigned char tm=0;

   tm = (PIN_Read(KEY0p)|(PIN_Read(KEY1p)<<1)|(PIN_Read(KEY2p)<<2));//读取各个按键状态并编码
   if(tm==0x07)
	{
	   return 0;
	}
//   while(tm == (PIN_Read(KEY0p)|(PIN_Read(KEY1p)<<1)|(PIN_Read(KEY2p)<<2)));//等待按键释放

   return  (~tm)&0X07;
}



