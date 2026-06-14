#ifndef __LQ_KEY_H_
#define __LQ_KEY_H_

//定义模块号
typedef enum
{
    KEY0 = 0,  //母板上按键0
    KEY1 = 1,
    KEY2 = 2,
    DSW0 = 3,    //母板上拨码开关0
    DSW1 = 4,
} KEYn_e;

typedef enum
{
    LOW = 0,  //按下
    HIGH = 1, //松开
    FAIL = 0xff, //错误
} KEYs_e;

typedef enum //
{
    NOKEYDOWN = 0, KEY0DOWN = 0x01,  //母板上按键0
    KEY1DOWN = 0x02,
    KEY2DOWN = 0x04,
    KEY01DOWN = 0x03,  //母板上按键0
    KEY02DOWN = 0x05,
    KEY12DOWN = 0x06,
    KEY012DOWN = 0x07,
    KEYError = 0xFF,
} KEYdown_e;
//定义的管脚要对应实际按键
#define KEY0p      P22_0  //母板上按键0
#define KEY1p      P22_1  //母板上按键1
#define KEY2p      P22_2  //母板上按键2
#define DSW0p      P33_9  //母板上拨码开关0,默认拨到下方是高电平，上方为低电平
#define DSW1p      P33_11 //母板上拨码开关1
#define REEDp      P15_4  //母板上干簧管
extern unsigned char KeyPress;
extern unsigned char Key_Number;
extern unsigned short  Key_Time;
extern unsigned int Set_Servo;
//初始化
void GPIO_KEY_Init (void);
void Timer_Key_Read(void);
unsigned char KEY_Read (KEYn_e KEYno);
unsigned char Key_Scan(void);
void Servo_Test_Form(void);
/*************************************************************************
*  函数名称：void Reed_Init(void)
*  功能说明：干簧管GPIO及中断初始化函数
*  备    注：   中断在那个核初始化，中断VECTABNUM号必须对应该cpu,否则不会调用中断函数
*          中断服务函数PIN_INT0_IRQHandler在LQ_GPIO.c中
*************************************************************************/
unsigned char KEY_Read_All (void);
void Test_GPIO_KEY (void);
void Test_ComKEY_Tft (void);
void Reed_Init(void);
#endif/* 0_APPSW_TRICORE_APP_LQ_ASC_H_ */
