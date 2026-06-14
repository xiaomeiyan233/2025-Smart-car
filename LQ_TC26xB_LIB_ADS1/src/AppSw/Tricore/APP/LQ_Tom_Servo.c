#include <IfxGtm_PinMap.h>
#include <LQ_GPIO_KEY.h>
#include <LQ_GPIO_LED.h>
#include <LQ_GTM.h>
#include <LQ_STM.h>
#include <LQ_UART.h>
#include <stdio.h>
#include "LQ_Tom_Servo.h"


ServoStruct servoStr;
unsigned int pwm_Servo = 0;//读取最终写入的pwm的值
unsigned int ServoPwm = 1500;//读取设置角度时的pwm的值
//       舵机PWM输出初始化
void SERVO_PWMInit(void)
{
    //新车 100hz
    TOM_PWM_InitConfig(TOMSERVO1,1765, SERVO_Frequency);//初始化P33_10 作为PWM输出口 频率100Hz 占空比 百分之(1500/TOM_PWM_MAX)*100
    servoStr.thresholdMiddle = 1235;
    //老车
    //TOM_PWM_InitConfig(TOMSERVO1,1255, SERVO_Frequency);//初始化P33_10 作为PWM输出口 频率100Hz 占空比 百分之(1500/TOM_PWM_MAX)*100
    //servoStr.thresholdMiddle = 1745;
    servoStr.thresholdLeft = SERVO_PWM_MAX_L;
    servoStr.thresholdRight = SERVO_PWM_MAX_R;
}
//       舵机输出PWM设置
void SERVO_SetPwmValue(unsigned int pwm) //范围为0~180度
{
    pwm = 3000 - pwm;  //左→右

    //if(pwm < SERVO_PWM_MIN)
        //pwm = SERVO_PWM_MIN;
    //else if(pwm > SERVO_PWM_MAX)
        //pwm = SERVO_PWM_MAX;
    TOM_PWM_SetDuty(TOMSERVO1, pwm, SERVO_Frequency);//设置频率功能已打开
}
/**
* @brief        舵机输出PWM设置（矫正后）
* @param        pwm：500~2500
**/
void SERVO_SetPwmValueCorrect(unsigned int pwm)//Servo 舵机，该程序没有进行舵机的初始化
{
    pwm = 3000 - pwm;  //左→右

    pwm -= servoStr.thresholdMiddle-SERVO_PWM_MIDDLE; //中值补偿

    uint16_t pwmMax = 3000 - servoStr.thresholdLeft;
    uint16_t pwmMin = 3000 - servoStr.thresholdRight;
    if(pwm < pwmMin)
        pwm = pwmMin;
    else if(pwm > pwmMax)
        pwm = pwmMax;

    pwm_Servo = pwm;
    TOM_PWM_SetDuty(TOMSERVO1, pwm, SERVO_Frequency);//设置频率功能已打开
}
/**
* @brief       舵机角度控制
**/
void SERVO_AngleControl(float angle)
{
    uint16_t pwm = 1500;
    angle = -angle;
    if(angle > SERVO_ANGLE_MAX)
        angle = SERVO_ANGLE_MAX;
    else if(angle < -SERVO_ANGLE_MAX)
        angle = -SERVO_ANGLE_MAX;

    if(angle >= 0)  //右转
        pwm = (float)angle/SERVO_ANGLE_MAX * (SERVO_PWM_MAX_R-servoStr.thresholdMiddle) + servoStr.thresholdMiddle;     //绝对角度计算
    else if(angle < 0)      //左转
        pwm = (float)angle/SERVO_ANGLE_MAX * (servoStr.thresholdMiddle - SERVO_PWM_MAX_L) + servoStr.thresholdMiddle;       //绝对角度计算

    ServoPwm = pwm;
    SERVO_SetPwmValue(pwm);
}
