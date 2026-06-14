#ifndef __LQ_LED_H_
#define __LQ_LED_H_


////定义的管脚要对应实际按键
#define LED0p      P10_6   //核心板上LED0 翠绿
#define LED1p      P10_5   //核心板上LED1 蓝灯
#define LED2p      P20_6   //母板上LED0
#define LED3p      P20_7   //母板上LED1

//定义模块号
typedef enum
{
    LED0=0,  //核心板上LED0，绿灯,用于与上位机通信状态的显示
    LED1=1,  //核心板上LED1,蓝灯，用于LED灯效
    LED2=2,  //母板上LED0,同步蜂鸣器一起亮
    LED3=3,   //母板上LED1,
    LEDALL=4
} LEDn_e;

typedef enum
{
    ON=0,  //亮
    OFF=1, //灭
    RVS=2, //反转
}LEDs_e;

//====================蜂鸣器=======================//
#define   BUZZER_PIN    P33_8

#define  BUZZER_ON        PIN_Write (BUZZER_PIN,0)
#define  BUZZER_OFF       PIN_Write (BUZZER_PIN,1)
#define  BUZZER_REV       PIN_Reverse(BUZZER_PIN)
/**
* @brief    蜂鸣器音效
**/
typedef enum
{
    BuzzerOk = 0,                       //确认提示音
    BuzzerWarnning,                     //报警提示音
    BuzzerSysStart,                     //开机提示音
    BuzzerDing,                         //叮=====(￣▽￣*)
    BuzzerFinish,                       //结束提示音
}BuzzerEnum;
/**
* @brief    蜂鸣器相关
**/
typedef struct
{
    unsigned int Times;                     //鸣叫次数
    unsigned int Counter;                   //计数器
    unsigned int Cut;                       //间隔时间
    unsigned char Enable;                   //使能标志
    unsigned char Silent;                   //是否禁用蜂鸣器
}BuzzerStruct;

extern BuzzerStruct buzzerStr;
extern unsigned int Delay_100ms[10];
extern unsigned int Count[10];

void BuzzerInit(void);
void GPIO_Timer(void);
void GPIO_Handle(void);
void GPIO_BuzzerEnable(BuzzerEnum buzzer);
//初始化测试程序
void GPIO_LED_Init(void);
void LED_Ctrl(LEDn_e LEDno, LEDs_e sta);
void Test_GPIO_LED(void);


#endif
