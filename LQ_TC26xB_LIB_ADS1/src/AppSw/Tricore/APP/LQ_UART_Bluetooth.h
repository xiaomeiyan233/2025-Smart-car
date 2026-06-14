#ifndef __LQ_UART_BT_H_
#define __LQ_UART_BT_H_

#include<stdbool.h>  //包含了bool
#include <stdio.h>

//==================================串口===================================//
#define   UART_Baudrate      115200            //串口波特率
#define   UART_RX            UART0_RX_P14_1    //串口接收引脚
#define   UART_TX            UART0_TX_P14_0    //串口发送引脚
#define   UART_Channel       UART0             //串口号
//========================================================================//
#define Wireless_UART_INDEX  UART1
#define Wireless_UART_BAUD   115200
#define Wireless_UART_RX     UART1_RX_P02_3
#define Wireless_UART_TX     UART1_TX_P02_2
//========================================================================//
#define USB_FRAME_HEAD               0x42                //USB通信序列帧头
#define USB_FRAME_LENMIN             4                   //USB通信序列字节最短长度
#define USB_FRAME_LENMAX             30                  //USB通信序列字节最长长度
//智能车接收信息地址
#define USB_ADDR_HEART               0x00                //监测软件心跳
#define USB_ADDR_CONTROL             0x01                //智能车控制
#define USB_ADDR_SPEEDMODE           0x10                //速控模式
#define USB_ADDR_SERVOTHRESHOLD      0x03                //舵机阈值
#define USB_ADDR_BUZZER              0x04                //蜂鸣器音效
#define USB_ADDR_LIGHT               0x05                //LED灯效
//智能车发送信息地址
#define USB_ADDR_KEYINPUT            0x06                //按键输入
#define USB_ADDR_BATTERY             0x0C                //电池信息
#define USB_ADDR_EdgeParam           0x09                //下位机发送上位机参数信息发送
#define USB_ADDR_INSPECTOR           0x0A                //智能车与上位机通信使能
#define USB_ADDR_SELFCHECK           0x0B                //智能车自检使能
#define USB_ADDR_SPEEDBACK           0x08                //车速信息反馈：m/s
//#define USB_ADDR_ANGLE               0x09                //车辆转过的角度
#define USB_ADDR_KD2                 0x07                //舵机pid的kd2
#define USB_ADDR_RING_ENTER          0x02                //环岛状态：进入
#define USB_ADDR_RING_INSIDE         0x03                //环岛状态：环中
#define USB_ADDR_RING_FINISH         0x08                //环岛状态：出环
#define USB_ADDR_CATER_ENTER         0x09                //餐饮店状态：进入


//EdgeBoard串口调试数据结构体
typedef struct
{
    bool receiveStart;                                      //数据接收开始
    uint8_t receiveIndex;                                   //接收序列
    bool receiveFinished;                                   //数据队列接收并校验完成
    uint8_t receiveBuff[USB_FRAME_LENMAX];                  //USB接收队列：临时接收
    uint8_t receiveBuffFinished[USB_FRAME_LENMAX];          //USB接收队列：校验成功
    uint16_t counter;                                       //计数器
    uint16_t counterDrop;                                   //掉线计数器

    uint16_t counterSend;                                   //自检数据发送计数器
    bool connected;                                         //上位机通信连接状态
    bool inspectorEnable;                                   //智能汽车自检软件连接使能
}UsbStruct;

//声明EdgeBoard串口通信数据结构体
extern UsbStruct usbStr;
extern float data;
typedef union
{
    uint8_t U8_Buff[2];
    uint16_t U16;
    int16_t S16;
}Bint16_Union;

typedef union
{
    uint8_t U8_Buff[4];
    float Float;
    unsigned long U32;
}Bint32_Union;

//Edge串口函数
void usbStr_Init(void);
void USB_Edgeboard_TransmitByte(uint8_t data);
//串口中断内嵌函数
void UART0_RX_Interrupt(void);
void USB_Edgeboard_Timr(void);
void USB_Edgeboard_Handle(void);
//自检软件数据发送
void USB_Edgeboard_KeyPress(uint16_t time);
void USB_Edgeboard_Send_ring_enter(float ring_enter);
void USB_Edgeboard_Send_ring_inside(float ring_inside);
void USB_Edgeboard_Send_ring_finish(float ring_finish);
void USB_Edgeboard_SendAngel(float angle);
void ring_ctrl(void);
void pid_ring(void);
float Turn_Filter(float turn);
void USB_Edgeboard_Send_cater_enter(float cater_enter);
void cater_ctrl(void);


int fputc(int ch, FILE *stream);
int fgetc(FILE *f);

#endif
