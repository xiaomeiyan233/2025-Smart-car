#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>
#include <LQ_UART.h>
#include "LQ_UART_Bluetooth.h"
#include "LQ_Tom_Servo.h"
#include "LQ_Atom_Motor.h"
#include "LQ_GPIO_KEY.h"
#include <LQ_I2C_MPU6050.h>
//USB结构体数据初始化
UsbStruct usbStr;
int ring_type = 0;
float data = 0;
float count = 0;
//============================================================================================================//
void usbStr_Init(void) //6 更改为宏定义
{
    UART_InitConfig(UART0_RX_P14_1,UART0_TX_P14_0,500000);
    usbStr.counter = 0;
    usbStr.receiveFinished = false;
    usbStr.receiveStart = false;
    usbStr.receiveIndex = 0;
    usbStr.connected = false;
    usbStr.inspectorEnable = false;
}

int Abs(int data)//求绝对值
{
    if(data <= 0)
        data = -data;
    return data;
}

/*滤波器，滤的是舵机的值*/
float Turn_Out_Filter(float turn_out)    //存储最近输入的六个舵机pwm值，带权重滤波，保证舵机更平滑
{
    float Turn_Out_Filtered = 0;  /*像下面这行给Pre1_Error数组赋初值是为了防止车起步时舵机抖一下*/
    static float Pre1_Error[6] = {SERVO_PWM_MIDDLE,SERVO_PWM_MIDDLE,SERVO_PWM_MIDDLE,SERVO_PWM_MIDDLE,SERVO_PWM_MIDDLE,SERVO_PWM_MIDDLE};
    if(turn_out>5000)
    {
       turn_out = SERVO_PWM_MIDDLE;
    }
    if(turn_out > SERVO_PWM_MAX_R && turn_out < 5000)
        turn_out = SERVO_PWM_MAX_R;
    if(turn_out < SERVO_PWM_MAX_L)
        turn_out = SERVO_PWM_MAX_L;
    Pre1_Error[5] = Pre1_Error[4];
    Pre1_Error[4] = Pre1_Error[3];
    Pre1_Error[3] = Pre1_Error[2];
    Pre1_Error[2] = Pre1_Error[1];
    Pre1_Error[1] = Pre1_Error[0];
    Pre1_Error[0] = turn_out;
    Turn_Out_Filtered = Pre1_Error[0]*0.2+Pre1_Error[1]*0.16+Pre1_Error[2]*0.16+Pre1_Error[3]*0.16+Pre1_Error[4]*0.16+Pre1_Error[5]*0.16;
    return Turn_Out_Filtered;
}

float Turn_Filter(float turn)
{
    if(turn>4500)
    {
        turn = SERVO_PWM_MIDDLE;
    }
    if(turn > SERVO_PWM_MAX_R && turn<4500)
        turn = SERVO_PWM_MAX_R;
    if(turn < SERVO_PWM_MAX_L)
        turn = SERVO_PWM_MAX_L;
    return turn;
}

/**
* @brief        USB-Edgeboard发送一个字节数据
**/
void USB_Edgeboard_TransmitByte(uint8_t data)
{
    UART_PutChar(UART0 , data);
}
//串口中断内嵌函数
void UART0_RX_Interrupt(void)
{
    uint8_t Uart1Res;
    Uart1Res = UART_GetChar(UART0);
    if(Uart1Res == USB_FRAME_HEAD && !usbStr.receiveStart)//监测帧头
    {
        usbStr.receiveStart = true;
        usbStr.receiveBuff[0] = Uart1Res;
        usbStr.receiveBuff[2] = USB_FRAME_LENMIN;
        usbStr.receiveIndex = 1;
    }
    else if(usbStr.receiveIndex == 2)   //接收帧长度
    {
        usbStr.receiveBuff[usbStr.receiveIndex] = Uart1Res;
        usbStr.receiveIndex++;
        if(Uart1Res > USB_FRAME_LENMAX || Uart1Res < USB_FRAME_LENMIN) //帧长错误
        {
            usbStr.receiveBuff[2] = USB_FRAME_LENMIN;
            usbStr.receiveIndex = 0;
            usbStr.receiveStart = false;
        }
    }
    else if(usbStr.receiveStart && usbStr.receiveIndex < USB_FRAME_LENMAX)
    {
        usbStr.receiveBuff[usbStr.receiveIndex] = Uart1Res;
        usbStr.receiveIndex++;
    }

    //接收帧完毕
    if((usbStr.receiveIndex >= USB_FRAME_LENMAX || usbStr.receiveIndex >= usbStr.receiveBuff[2]) && usbStr.receiveIndex > USB_FRAME_LENMIN)
    {
        uint8_t check = 0;
        uint8_t length = USB_FRAME_LENMIN;

        length = usbStr.receiveBuff[2];
        for(int i=0;i<length-1;i++)
        {
            check += usbStr.receiveBuff[i];
        }
        if(check == usbStr.receiveBuff[length-1])//校验位
        {
            for(int n=0;n<USB_FRAME_LENMAX;n++)
            {
                usbStr.receiveBuffFinished[n] = usbStr.receiveBuff[n];
            }
            usbStr.receiveFinished = true;
            //LED_Ctrl(LED0, RVS); //每正常接收2次，LED0闪烁1次
            //智能车控制指令特殊处理（保障实时性）
            if(USB_ADDR_CONTROL  == usbStr.receiveBuffFinished[1])
            {
                Bint16_Union bint16_Union;
                Bint32_Union bint32_Union;
                for(int i=0;i<4;i++)
                    bint32_Union.U8_Buff[i] = usbStr.receiveBuffFinished[3+i];

                bint16_Union.U8_Buff[0] = usbStr.receiveBuffFinished[7];
                bint16_Union.U8_Buff[1] = usbStr.receiveBuffFinished[8];
                //SERVO_SetPwmValueCorrect();
                //if(icarStr.ring_inside ==0&&icarStr.ring_finish == 0)//进入环岛不接收上位机发送的舵机pwm
                if(icarStr.ring_inside == 0 && icarStr.ring_enter == 0 && icarStr.ring_finish == 0 && icarStr.cater_enter == 0)//不在环中清零角度积累
                {
                    angle_sum = 0;
                }
                //if(angle_sum < 210 && angle_sum  > -210)//出环阶段停止接收舵机角度
                //{
                    count++;
                    if(count ==30)
                    {
                        data++;
                        count = 0;
                    }
                      SERVO_SetPwmValue(Turn_Out_Filter(bint16_Union.U16-265-(icarStr.kd2*gyroz_s)));
                      icarStr.ServoPwmSet = Turn_Out_Filter(bint16_Union.U16-265-(icarStr.kd2*gyroz_s));         //方向
               // }
                icarStr.SpeedSet = bint32_Union.Float;              //速度
            }
            if(!usbStr.connected)//上位机初次连接通信
            {
                GPIO_BuzzerEnable(BuzzerOk);
                usbStr.connected = true;
            }
            usbStr.counterDrop = 0;
        }
        usbStr.receiveIndex = 0;
        usbStr.receiveStart = false;
    }
}
/**
* @brief   监测软件线程控制器
**/
void USB_Edgeboard_Timr(void)
{
    if(usbStr.connected)//USB通信掉线检测
    {
        usbStr.counterDrop++;
        if(usbStr.counterDrop >3000)//3s
        {
            usbStr.connected = false;
            usbStr.inspectorEnable = false;
            icarStr.selfcheckEnable = false;
       }

        if(usbStr.inspectorEnable)
        {
            usbStr.counterSend++;
        }
    }
}
/**
* @brief        USB通信处理函数
**/
void USB_Edgeboard_Handle(void)
{
    if(usbStr.receiveFinished)                                                              //接收成功
    {
        usbStr.receiveFinished = false;
        if(usbStr.receiveBuffFinished[1] & 0x80)    //读数据
        {
            uint8_t Addr = (uint8_t)(usbStr.receiveBuffFinished[1] & 0x7F);
            switch(Addr)
            {
                case USB_ADDR_BATTERY :             //电池信息
                    break;
            }
        }
        else //写数据到下位机
        {
            switch(usbStr.receiveBuffFinished[1])
            {
                case USB_ADDR_BUZZER :      //蜂鸣器音效
                    if(usbStr.receiveBuffFinished[3] == 1)          //OK
                        GPIO_BuzzerEnable(BuzzerOk);
                    else if(usbStr.receiveBuffFinished[3] == 2)     //Warnning
                        GPIO_BuzzerEnable(BuzzerWarnning);
                    else if(usbStr.receiveBuffFinished[3] == 3)     //Finish
                        GPIO_BuzzerEnable(BuzzerFinish);
                    else if(usbStr.receiveBuffFinished[3] == 4)     //Ding
                        GPIO_BuzzerEnable(BuzzerDing);
                    else if(usbStr.receiveBuffFinished[3] == 5)     //SystemStart
                        GPIO_BuzzerEnable(BuzzerSysStart);
                    break;
                case USB_ADDR_KD2 :
                {
                    Bint32_Union bint32_Union;
                    for(int i = 0; i < 4; i++)
                      {
                        bint32_Union.U8_Buff[i] = usbStr.receiveBuffFinished[3 + i];
                      }
                    float kd2 = bint32_Union.Float;
                   // 将接收到的 kd2 值保存到相应变量中
                   icarStr.kd2 = kd2;
                     break;
                }
                case USB_ADDR_CATER_ENTER:
                {
                    Bint32_Union bint32_Union;
                    for(int i = 0; i < 4; i++)
                      {
                        bint32_Union.U8_Buff[i] = usbStr.receiveBuffFinished[3 + i];
                      }
                    icarStr.cater_enter = bint32_Union.Float;
                     break;
                }
                case USB_ADDR_RING_ENTER:
                {
                    Bint32_Union bint32_Union;
                    for(int i = 0; i < 4; i++)
                      {
                        bint32_Union.U8_Buff[i] = usbStr.receiveBuffFinished[3 + i];
                      }
                    icarStr.ring_enter = bint32_Union.Float;
                     break;
                }
                case USB_ADDR_RING_INSIDE:
                {
                    Bint32_Union bint32_Union;
                    for(int i = 0; i < 4; i++)
                      {
                        bint32_Union.U8_Buff[i] = usbStr.receiveBuffFinished[3 + i];
                      }
                    icarStr.ring_inside = bint32_Union.Float;
                     break;
                }
                case USB_ADDR_RING_FINISH:
                {
                    Bint32_Union bint32_Union;
                    for(int i = 0; i < 4; i++)
                      {
                        bint32_Union.U8_Buff[i] = usbStr.receiveBuffFinished[3 + i];
                      }
                    icarStr.ring_finish = bint32_Union.Float;
                     break;
                }
                case USB_ADDR_SPEEDMODE:        //速控模式切换
                    if(usbStr.receiveBuffFinished[3] == 1)    //开环模式
                        motorStr.CloseLoop = false;
                    else
                        motorStr.CloseLoop = true;

                    icarStr.SpeedSet = 0;
                    break;
                case USB_ADDR_INSPECTOR :           //上位机发送这个信号后，下位机可以向上位机发送信息
                    usbStr.inspectorEnable = true;
                    break;
            }

        }
    }
    //当按键按下时，系统向上位机发送按键信号
    if(Key_Time == 1001)
    {
        USB_Edgeboard_KeyPress(Key_Time);
        Key_Time = 0;
    }
    //-----------------------[上位机参数数据发送]-----------------------------
    if(usbStr.inspectorEnable && usbStr.connected && usbStr.counterSend > 150)//150ms,这里发送的不止一次
    {
        usbStr.counterSend = 0;
//        usbStr.inspectorEnable = 0;//如果置零这个标志，就只发一次数据
    }

}

/**
* @brief        发送按键响应信息
* @param        time: 按下时长/ms
**/
void USB_Edgeboard_KeyPress(uint16_t time)
{
    if(time<1000)
        return;

    Bint16_Union bint16_Union;
    uint8_t check = 0;
    uint8_t buff[8];
    buff[0] = 0x42; //帧头
    buff[1] = USB_ADDR_KEYINPUT; //地址
    buff[2] = 0x06; //帧长

    bint16_Union.U16 = time;
    buff[3] = bint16_Union.U8_Buff[0];
    buff[4] = bint16_Union.U8_Buff[1];

    for(int i=0;i<5;i++)
        check += buff[i];

    buff[5] = check;

    for(int i=0;i<8;i++)
        USB_Edgeboard_TransmitByte(buff[i]);
}

//环岛相关代码

void ring_ctrl(void)
{
    if(icarStr.ring_enter == 1)//上位机向下位机发送入环标志
    {//加判断角速度达到一定值才进inside
        if(angle_sum > 35 || angle_sum < -30)
        {
            icarStr.ring_inside = 1;
            for(int i = 0;i<20;i++)
            {
                USB_Edgeboard_Send_ring_inside(icarStr.ring_inside);
            }
            icarStr.ring_enter = 0;
        }
        /*
        if(gyroz_s > 10)//左环岛
        {
            ring_type = 0;
            icarStr.ring_inside = 1;
            for(int i = 0;i<20;i++)
            {
                USB_Edgeboard_Send_ring_inside(icarStr.ring_inside);
            }
            icarStr.ring_enter = 0;
         }
        if(gyroz_s < -10)//右环岛
        {
            ring_type = 1;
            icarStr.ring_inside = 1;
            for(int i = 0;i<20;i++)
            {
                USB_Edgeboard_Send_ring_inside(icarStr.ring_inside);
            }
            icarStr.ring_enter = 0;
         }
         */
    }
    if(icarStr.ring_inside == 1)
    {
        if(angle_sum > 210 && angle_sum < 320)//最后出环阶段陀螺仪辅助出环 300
        {
            //SERVO_SetPwmValue(1080);//940
        }
        if(angle_sum >= 320)
        {
            icarStr.ring_finish = 1;
            icarStr.ring_inside = 0;
        }
        if(angle_sum < -210 && angle_sum > -320)
        {
            //SERVO_SetPwmValue(1390);//1530
        }
        if(angle_sum <= -320)
        {
            icarStr.ring_finish = 1;
            icarStr.ring_inside = 0;
        }
        /*
        pid_ring();
        if(Abs(angle_sum)>350)
        {
            icarStr.ring_finish = 1;
            icarStr.ring_inside = 0;
        }
        */
    }
    if(icarStr.ring_finish == 1)
    {
        for(int i = 0;i<20;i++)
        {
            USB_Edgeboard_Send_ring_finish(icarStr.ring_finish);
        }

        icarStr.ring_finish = 0;
    }
}

void cater_ctrl(void)
{
    if(icarStr.cater_enter == 1)//汉堡在左侧，右餐饮店
    {
        if(angle_sum < -35)
        {
            for(int i = 0;i < 5; i++)
            {
                USB_Edgeboard_Send_cater_enter(3);
            }
            icarStr.cater_enter = 0;
        }

    }
    if(icarStr.cater_enter == 2)//汉堡在右侧，左餐饮店
    {
        if(angle_sum > 35)
        {
            for(int i = 0;i < 5; i++)
            {
                USB_Edgeboard_Send_cater_enter(3);
            }
            icarStr.cater_enter = 0;
        }
    }
}

void pid_ring(void)
{
    icarStr.SpeedSet = 0.5;
    float r = 0.16;//单位m
    float kp = 3,kd = 0.4,output = 0;
    float target =(icarStr.SpeedSet/r)*57.3 ,e = 0,ec = 0,errlast = 0;//target为每秒转多少°
    if(ring_type == 0)//左环岛
    {
       e = target - gyroz_s;
       ec = e - errlast;
       output = kp*e+kd*ec;
       SERVO_SetPwmValue(Turn_Filter(SERVO_PWM_MIDDLE - output));
       icarStr.ServoPwmSet = Turn_Filter(SERVO_PWM_MIDDLE - output);         //方向
    }
    if(ring_type == 1)//右环岛
    {
       e = -target - gyroz_s;
       ec = e - errlast;
       output = kp*e+kd*ec;
       SERVO_SetPwmValue(Turn_Filter(SERVO_PWM_MIDDLE - output));
       icarStr.ServoPwmSet = Turn_Filter(SERVO_PWM_MIDDLE - output);         //方向
    }
    errlast = e;
}

void USB_Edgeboard_Send_ring_enter(float ring_enter)
{
    Bint32_Union bint32_Union; // 用于把 float 数据转换成4字节数据
    uint8_t check = 0;
    uint8_t buff[8]; // 整个数据帧共8个字节

    // 填入帧头、地址和帧长度（整体帧长为8字节：1头+1地址+1长度+4 payload+1校验）
    buff[0] = 0x42;              // USB_FRAME_HEAD，一般为0x42
    buff[1] = USB_ADDR_RING_ENTER;
    buff[2] = 0x08;              // 帧长：8个字节

    // 将 float 数据转换为4字节
    bint32_Union.Float = ring_enter;
    buff[3] = bint32_Union.U8_Buff[0];
    buff[4] = bint32_Union.U8_Buff[1];
    buff[5] = bint32_Union.U8_Buff[2];
    buff[6] = bint32_Union.U8_Buff[3];

    // 计算校验位：校验位为前 (帧长-1) 个字节的累加和
    for (int i = 0; i < 7; i++)
    {
        check += buff[i];
    }
    buff[7] = check;

    // 发送数据帧的每个字节
    for (int i = 0; i < 8; i++)
    {
        USB_Edgeboard_TransmitByte(buff[i]);
    }
}

void USB_Edgeboard_Send_ring_inside(float ring_inside)
{
    Bint32_Union bint32_Union; // 用于把 float 数据转换成4字节数据
    uint8_t check = 0;
    uint8_t buff[8]; // 整个数据帧共8个字节

    // 填入帧头、地址和帧长度（整体帧长为8字节：1头+1地址+1长度+4 payload+1校验）
    buff[0] = 0x42;              // USB_FRAME_HEAD，一般为0x42
    buff[1] = USB_ADDR_RING_INSIDE;
    buff[2] = 0x08;              // 帧长：8个字节

    // 将 float 数据转换为4字节
    bint32_Union.Float = ring_inside;
    buff[3] = bint32_Union.U8_Buff[0];
    buff[4] = bint32_Union.U8_Buff[1];
    buff[5] = bint32_Union.U8_Buff[2];
    buff[6] = bint32_Union.U8_Buff[3];

    // 计算校验位：校验位为前 (帧长-1) 个字节的累加和
    for (int i = 0; i < 7; i++)
    {
        check += buff[i];
    }
    buff[7] = check;

    // 发送数据帧的每个字节
    for (int i = 0; i < 8; i++)
    {
        USB_Edgeboard_TransmitByte(buff[i]);
    }
}

void USB_Edgeboard_Send_ring_finish(float ring_finish)
{
    Bint32_Union bint32_Union; // 用于把 float 数据转换成4字节数据
    uint8_t check = 0;
    uint8_t buff[8]; // 整个数据帧共8个字节

    // 填入帧头、地址和帧长度（整体帧长为8字节：1头+1地址+1长度+4 payload+1校验）
    buff[0] = 0x42;              // USB_FRAME_HEAD，一般为0x42
    buff[1] = USB_ADDR_RING_FINISH;
    buff[2] = 0x08;              // 帧长：8个字节

    // 将 float 数据转换为4字节
    bint32_Union.Float = ring_finish;
    buff[3] = bint32_Union.U8_Buff[0];
    buff[4] = bint32_Union.U8_Buff[1];
    buff[5] = bint32_Union.U8_Buff[2];
    buff[6] = bint32_Union.U8_Buff[3];

    // 计算校验位：校验位为前 (帧长-1) 个字节的累加和
    for (int i = 0; i < 7; i++)
    {
        check += buff[i];
    }
    buff[7] = check;

    // 发送数据帧的每个字节
    for (int i = 0; i < 8; i++)
    {
        USB_Edgeboard_TransmitByte(buff[i]);
    }
}

void USB_Edgeboard_Send_cater_enter(float cater_enter)
{
    Bint32_Union bint32_Union; // 用于把 float 数据转换成4字节数据
    uint8_t check = 0;
    uint8_t buff[8]; // 整个数据帧共8个字节

    // 填入帧头、地址和帧长度（整体帧长为8字节：1头+1地址+1长度+4 payload+1校验）
    buff[0] = 0x42;              // USB_FRAME_HEAD，一般为0x42
    buff[1] = USB_ADDR_CATER_ENTER;
    buff[2] = 0x08;              // 帧长：8个字节

    // 将 float 数据转换为4字节
    bint32_Union.Float = cater_enter;
    buff[3] = bint32_Union.U8_Buff[0];
    buff[4] = bint32_Union.U8_Buff[1];
    buff[5] = bint32_Union.U8_Buff[2];
    buff[6] = bint32_Union.U8_Buff[3];

    // 计算校验位：校验位为前 (帧长-1) 个字节的累加和
    for (int i = 0; i < 7; i++)
    {
        check += buff[i];
    }
    buff[7] = check;

    // 发送数据帧的每个字节
    for (int i = 0; i < 8; i++)
    {
        USB_Edgeboard_TransmitByte(buff[i]);
    }
}

//-------------------------------------------------------------------------     // printf 重定向
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      重定向printf 到串口
//  参数说明      ch      需要打印的字节
//  参数说明      stream  数据流
//  备注信息       此函数由编译器自带库里的printf所调用
//-------------------------------------------------------------------------------------------------------------------
int fputc(int ch, FILE *stream)
{
    UART_PutChar(Wireless_UART_INDEX, (char)ch);

    return(ch);
}
//-------------------------------------------------------------------------------------------------------------------
//  函数简介      重定向printf 到串口
//  参数说明      ch      需要打印的字节
//  参数说明      stream  数据流
//  备注信息       此函数由编译器自带库里的printf所调用
//-------------------------------------------------------------------------------------------------------------------
int fgetc(FILE *f)
{
    return UART_GetChar(Wireless_UART_INDEX);
}
