/*QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ
----------------------------------------------------------------
推荐GPT12模块，实现正交解码增量编码器（兼容带方向编码器）信号采集
En_Input      P10_3
En_Dir        P10_1
-----------------------------------------------------------------
电感电压采集模块
推荐使用AN0-7，共八路ADC，可以满足chirp声音信号及电磁车电感电压采集；
电压采集         ADC7
提醒：在开发板与下载器连接时，会对电量的读取有影响，此时蜂鸣器可能会响，拔掉下载器重新复位即可。
-----------------------------------------------------------------
默认电机接口
使用GTM模块，ATOM四个通道可产生4*8共32路PWM，而且各自频率和占空比可调，推荐使用ATOM0的0-7通道；
Motor1_PWM         P23_1
Motor1_Dir         P21_2
-----------------------------------------------------------------
默认舵机接口
使用GTM模块，ATOM四个通道可产生4*8共32路PWM，而且各自频率和占空比可调，推荐使用ATOM2；
Servo1_Pwm        P33_10
-----------------------------------------------------------------
 默认屏幕显示接口，用户可以使用TFT或者OLED模块 //暂时屏蔽
TFTSPI_CS       P20_13     // CS管脚 默认拉低，可以不用
TFTSPI_SCK      P20_14    // SPI SCK管脚
TFTSPI_SDI      P20_11     // SPI MOSI管脚
TFTSPI_DC       P20_12     // D/C管脚
TFTSPI_RST      P20_10     // RESET管脚
----------------------------------------------------------------
默认按键接口                 //暂时屏蔽按键1,2，按键时间改为50ms~100ms
KEY0p         P22_0      按键0   长按100ms后，向上位机发送多次特殊信号
KEY1p         P22_1      按键1   按下后向EEPROM中存储数据 有蜂鸣器提示
KEY2p         P22_2      按键2   按下后从EEPROM中读取数据 有蜂鸣器提示
-----------------------------------------------------------------
与Edge_Board通信串口：UART0
UART_Baudrate      115200            //串口波特率
UART_RX            UART0_RX_P14_1    //串口接收引脚
UART_TX            UART0_TX_P14_0    //串口发送引脚
UART_Channel       UART0             //串口号
========================================================================//
与调参助手通信串口：UART1               //暂时屏蔽
Wireless_UART_INDEX  UART1
Wireless_UART_BAUD   115200
Wireless_UART_RX     UART1_RX_P02_3
Wireless_UART_TX     UART1_TX_P02_2

提：应保证与Edge_Board通信串口的接收中断优先级高于与调参助手通信串口，而且两者的接发应互不影响
-----------------------------------------------------------------
默认LED接口
LED0         P10_6    伴随串口接收数据而闪烁
LED1         P10_5
LED2         P20_6    伴随蜂鸣器的鸣叫而闪烁
LED3         P20_7    伴随着程序的运行而闪烁
-----------------------------------------------------------------
默认蜂鸣器接口    //改变一下蜂鸣器的响声
BEEPp         P33_8      蜂鸣器接口
蜂鸣器使用情况
电池电量不足提醒音效                     BuzzerWarnning//报警提示音
串口每次连接成功或掉线再次连接              BuzzerOk//确认提示音
按键0 Key0按下时间大于1000ms时音效       BuzzerDing//叮=====(￣▽￣*)   //去掉
上位机调节速控模式（开环与闭环切换）          BuzzerDing//叮=====(￣▽￣*)
上电或复位音效                         BuzzerSysStart//开机提示音
EEPROM存取数据完成后音效                BuzzerFinish//完成提示音       //去掉
EEPROM读出数据完成后音效                BuzzerDing//叮=====(￣▽￣*)   //去掉
-----------------------------------------------------------------
ICM20602陀螺仪接口
-----------------------------------------------------------------
EEPROM       //暂时不用
无需初始化，直接调用函数就可以使用TC264内部自带的EEPROM了
使用前记得清空扇区原有的数据
-----------------------------------------------------------------
Icar        // 无用
-----------------------------------------------------------------
定时器
STM_InitConfig(STM0, STM_Channel_0, 1000);/
初始化CCU6模块的定时器0的通道0定时时间为1000us
定时器延时Delay_100m[n]使用情况
Delay_100m[2] 在主程序的TFT屏幕显示中被使用，每200ms显示一次
Delay_100m[3] 在蜂鸣器定时线程函数中被使用，用于在蜂鸣器鸣叫时闪烁
有两个CCU6模块  每个模块有两个独立定时器  触发定时器中断
推荐使用CCU6模块，STM用作系统时钟或者延时；
QQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQQ*/
#include <include.h>//各个模块的头文件
#include <IfxCpu.h>
#include <IfxScuCcu.h>
#include <IfxScuWdt.h>
#include <IfxStm.h>
#include <IfxStm_reg.h>
#include <LQ_CCU6.h>
#include <LQ_GPIO_KEY.h>
#include <LQ_GPIO_LED.h>
#include <LQ_SOFTI2C.h>
#include <LQ_UART.h>
#include <LQ_TFT18.h>
#include <Main.h>
#include <LQ_STM.h>
#include "LQ_Atom_Motor.h"
#include "LQ_Tim_InputCature.h"
#include <LQ_SOFTI2C.h>
#include "LQ_ICM20602.h"
#include <LQ_I2C_MPU6050.h>
#include "LQ_UART_Bluetooth.h"

App_Cpu0 g_AppCpu0; // brief CPU 0 global data
IfxCpu_mutexLock mutexCpu0InitIsOk = 1;   // CPU0 初始化完成标志位
volatile char mutexCpu0TFTIsOk=0;         // CPU1 0占用/1释放 TFT

/*************************************************************************
*  函数名称：int core0_main (void)
*  功能说明：CPU0主函数
*************************************************************************/
int core0_main (void)
{
    char Speed_Show[32];
    float mode = 0;
	// 关闭CPU总中断
	IfxCpu_disableInterrupts();
	// 关闭看门狗，如果不设置看门狗喂狗需要关闭
	IfxScuWdt_disableCpuWatchdog(IfxScuWdt_getCpuWatchdogPassword());
	IfxScuWdt_disableSafetyWatchdog(IfxScuWdt_getSafetyWatchdogPassword());
	// 读取总线频率
	g_AppCpu0.info.pllFreq = IfxScuCcu_getPllFrequency();
	g_AppCpu0.info.cpuFreq = IfxScuCcu_getCpuFrequency(IfxCpu_getCoreIndex());
	g_AppCpu0.info.sysFreq = IfxScuCcu_getSpbFrequency();
	g_AppCpu0.info.stmFreq = IfxStm_getFrequency(&MODULE_STM0);
	//go to语句
	end:
	icarStr.kd2 = 0;
	gyroz_s = 0;
	angle_sum = 0;
	data = 0;
	//陀螺仪初始化
	IIC_Init();
	MPU6050_Init();
	//TFT屏初始化
    TFTSPI_Init(1);               // TFT1.8初始化0:横屏  1：竖屏
    TFTSPI_CLS(u16BLACK);         // 清屏，显示黑色屏幕
	//定时器中断初始化
	STM_Timer_Init();
	//蜂鸣器初始化
	BuzzerInit();
	//舵机初始化
	SERVO_PWMInit();
	//电机，编码器初始化
	MotorStr_Init();
	Encoder_Init();
	// 按键初始化
	GPIO_KEY_Init();
	//LED初始化
	GPIO_LED_Init();
    LED_Ctrl(LEDALL, OFF);
	// 串口P14.0管脚输出,P14.1输入，波特率115200
	usbStr_Init();
	//智能车综合处理函数初始化
	ICAR_Init();
	//PID初始化
	PID_Init();
	//ADC电池电压监测
	ADC_Detection_Init();
	// 开启CPU总中断
	IfxCpu_enableInterrupts();
	// 通知CPU1，CPU0初始化完成
	IfxCpu_releaseMutex(&mutexCpu0InitIsOk);
	// 切记CPU0,CPU1...不可以同时开启屏幕显示，否则冲突不显示
	mutexCpu0TFTIsOk=0;         // CPU1： 0占用/1释放 TFT
	delayms(100);
    // 所有含有中断的测试都默认在CPU0中执行
	GPIO_BuzzerEnable(BuzzerSysStart);  //开机音效
	/***********
	 * 程序分为两大状态 调试状态和运行状态   //去掉
	 * 调试状态时 下位机正常通过串口0 与上位机进行通信，此时上位机应该时刻注意接收按键信号1，接收到按键信号1后
	 * ，上位机应给下位机发送“发送使能信号”（多次发送并非只发一次），下位机开始向上位机发送'参数和PID值'。
	 * 下位机同时通过串口1与调试软件通信，通信的结果可以通过按按键1进行保存，并通过按键2进行读取。
	 * 下位机发送按键信号后，即进入运行状态。
	 * 运行状态时 下位机断开与调试串口UART1的联系（即不再通过无线串口发送和接收数据），此时按键功能仍存在。当下位机与上位机断开连接时，车速为零，车停止运动。
	 * 调试状态和运行状态可随时转变，通过按键0按下1秒后切换（运行状态切回调试状态时，上位机也会接收到按键信号2并切回到等待接受按键信号1的状态）。
	 * 因为下位机的状态在复位后会置位为调试状态，为了不影响上位机则上位机永远只读一个状态信号。
	 * **********/
    while (1)	//主循环
    {
        if(Key_Number == 1)//按键向上位机发送发车指令
        {
            Key_Number = 0;
            //if(mode == 0)//模式0发送发车指令
            //{
               Key_Time = 1001;
            //}
            //if(mode == 1)//模式1修改kd2
            //{
                //icarStr.kd2+=0.01;
            //}
        }
        else if(Key_Number == 2)//最下面按键切换模式
        {
            Key_Number = 0;
            //mode++;
            //if(mode >= 3)
            //{
               //mode = 0;
            //}
            goto end;
        }
        /*
        else if(Key_Number == 3)
        {
            Key_Number = 0;
            if(mode == 1)
            {
                icarStr.kd2-=0.01;
            }
            if(mode == 2)
            {
                goto end;
            }
        }*/
        if(Delay_100ms[2] == 1)//200ms读取一次
        {
            USB_Edgeboard_Handle();//为了保证陀螺仪读取和发送同时进行
            LED_Ctrl(LED3, RVS);
            ring_ctrl();
            cater_ctrl();
            Delay_100ms[2] = 0;
            ADC_Voltage_Detection();

            sprintf(Speed_Show, "Z:%6.1f", gyroz_s);//陀螺仪z轴，单位°/s
            TFTSPI_P8X16Str(1,1,Speed_Show,u16WHITE,u16BLACK);

            sprintf(Speed_Show, "angle_sum:%3.1f", angle_sum);//角度积累
            TFTSPI_P8X16Str(1,2,Speed_Show,u16WHITE,u16BLACK);

            sprintf(Speed_Show, "kd2:%2f", icarStr.kd2);//左转陀螺仪参数
            TFTSPI_P8X16Str(1,3,Speed_Show,u16WHITE,u16BLACK);

            sprintf(Speed_Show, "data:%8f", data);//下位机实际读到的数据量
            TFTSPI_P8X16Str(1,4,Speed_Show,u16WHITE,u16BLACK);

            sprintf(Speed_Show, "Connect:%d", usbStr.connected);//上下位机连接状态
            TFTSPI_P8X16Str(1,5,Speed_Show,u16WHITE,u16BLACK);
        }
        GPIO_Handle();                  //GPIO控制：LED/蜂鸣器
//        ICAR_Handle();                  //智能车控制
        USB_Edgeboard_Handle();         //USB通信控制
    }
}
