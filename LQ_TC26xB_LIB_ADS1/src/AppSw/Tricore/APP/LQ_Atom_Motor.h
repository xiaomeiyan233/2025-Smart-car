#ifndef __LQ_MT_H_
#define __LQ_MT_H_
//舵机两路引脚代号
#define ATOMSERVO1       IfxGtm_ATOM2_0_TOUT32_P33_10_OUT
#define ATOMSERVO2       IfxGtm_ATOM2_5_TOUT35_P33_13_OUT
//电机八路引脚代号
//#define ATOMPWM0Pin    IfxGtm_ATOM0_2_TOUT107_P10_5_OUT //定义PWM输出管脚，通过P10.5输出PWM控制蓝灯呼吸
#define ATOMPWM0         IfxGtm_ATOM0_6_TOUT42_P23_1_OUT
#define ATOMPWM1         IfxGtm_ATOM0_5_TOUT40_P32_4_OUT
#define ATOMPWM2         IfxGtm_ATOM0_0_TOUT53_P21_2_OUT
#define ATOMPWM3         IfxGtm_ATOM0_4_TOUT50_P22_3_OUT
#define ATOMPWM4         IfxGtm_ATOM0_2_TOUT55_P21_4_OUT
#define ATOMPWM5         IfxGtm_ATOM0_1_TOUT54_P21_3_OUT
#define ATOMPWM6         IfxGtm_ATOM0_7_TOUT64_P20_8_OUT
#define ATOMPWM7         IfxGtm_ATOM0_3_TOUT56_P21_5_OUT

#define ATOMPWM10        IfxGtm_ATOM2_6_TOUT62_P20_6_OUT  //新引脚
#define  Motor1_PWM         P23_1        // P20_6 //
#define  Motor1_Dir         P21_2        // P20_7 //
#define  MOTOR_PWM_MAX              5000       //OCR=95%,禁止满占空比输出，造成MOS损坏
#define  MOTOR_PWM_MIN              -5000       //OCR=95%
#define  MOTOR_SPEED_MAX            5.0f       //电机最大转速(m/s) (0.017,8.04)
#define  PI                         3.141593f   //π
#define  MOTOR_CONTROL_CYCLE        0.01f       //电机控制周期T：10ms
#define  MOTOR_Frequenty            12500       //电机频率为12KHz
/******************************************************************************/
#define VV_DEADLINE             2                       //速度PID，设置死区范围，消抖，静止强硬程度
#define PID_VKP                 82.5f                    //PID标定值,KP范围: -25<Kp<25
#define PID_VKI                 17.5f                    //PID标定值,Ki范围: -15<Ki<15
#define PID_VKD                 0.0f                   //PID标定值,Kd范围: -25<Kd<25
//=============================电机，舵机控制==================================//
typedef struct   //电机具体参数设置
{
    float ReductionRatio ;                      //电机减速比
    float EncoderLine ;                         //编码器线数=光栅数16*4
    signed int EncoderValue;                    //编码器实时速度，有符号的16位值
    int CloseLoop;                              //开环模式
    int Counter;                                //线程计数器
    float DiameterWheel;                        //轮子直径：mm

}MotorStruct;
//===========================智能车相关控制==========================//
typedef struct                              //[智能车驱动主板]
{
    float Voltage;                          //电池电压
    unsigned char Electricity;              //电池电量百分比：0~100
    float SpeedSet;                         //电机目标速度：m/s
    float SpeedFeedback;                    //电机模型实测速度：m/s
    float SpeedMaxRecords;                  //测试记录最高速
    unsigned int ServoPwmSet;               //舵机PWM设置
    float kd2;
    float ring_enter;                     //环岛状态
    float ring_inside;
    float ring_finish;
    float cater_enter;                     //餐饮店状态



    unsigned int counterKeyA;               //按键模式A计数器
    unsigned char keyPressed;               //按键按下
    unsigned char sprintEnable;             //闭环冲刺使能
    unsigned int counterSprint;             //闭环冲刺时间
    unsigned int errorCode;                 //错误代码

    unsigned char selfcheckEnable;          //智能车自检使能
    unsigned int counterSelfcheck;          //自检计数器
    unsigned char timesSendStep;            //发送超时数据次数
    unsigned int counterModuleCheck;        //自检计数器
//    SelfcheckEnum selfcheckStep;          //自检步骤
    unsigned char speedSampleStep;          //速度采样步骤
}IcarStruct;

//========================PID控制===========================//

typedef struct
{
    float vi_Ref;                       //速度PID，速度设定值
    float vi_FeedBack;                  //速度PID，速度反馈值
    float vi_PreError;                  //速度PID，速度误差,vi_Ref - vi_FeedBack
    float vi_PreDerror;                 //速度PID，前一次，速度误差之差，d_error-PreDerror;
    float v_Kp;                         //比例系数，Kp = Kp
    float v_Ki;                         //积分系数，Ki = Kp * ( T / Ti )
    float v_Kd;                         //微分系数，Kd = KP * Td * T
    float vl_PreU;                      //PID输出值
}PIDStruct;

extern MotorStruct motorStr;
extern PIDStruct pidStr;
extern IcarStruct icarStr;
extern float Data;

void Motor_Encoder_Test(void);

void MotorStr_Init(void);
void MOTOR_SetPwmValue(signed int pwm);
void MOTOR_ControlLoop(float speed);
void MOTOR_Timer(void);
void ICAR_Init(void);
void PID_Init(void);
signed int PID_MoveCalculate(PIDStruct *pp);
//void LQ_Atom_Motor_8chPWM(void);
//void LQ_ATom_Servo_2chPWM(void);
//void LQ_Atom_8chPWM(void);

#endif
