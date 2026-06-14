#include <LQ_GPIO_KEY.h>
#include <LQ_GPIO_LED.h>
#include <LQ_GPIO.h>
#include <LQ_GTM.h>
#include <LQ_STM.h>
#include <LQ_UART.h>
#include <stdio.h>
#include <LQ_GPT12_ENC.h>
#include "LQ_Atom_Motor.h"
#include "LQ_Tim_InputCature.h"
#include "LQ_UART_Bluetooth.h"

MotorStruct motorStr;
IcarStruct icarStr;
PIDStruct pidStr;
float Data;

void Motor_Encoder_Test(void)
{
    static float Set_Speed = 0.0;
    if(Key_Number == 1)
    {
        Key_Number = 0;
        Set_Speed += 0.1;
        if(Set_Speed > MOTOR_SPEED_MAX)
            Set_Speed = MOTOR_SPEED_MAX;
        else if(Set_Speed < -MOTOR_SPEED_MAX)
            Set_Speed = -MOTOR_SPEED_MAX;
        icarStr.SpeedSet = Set_Speed;
    }
    else if(Key_Number == 2)
    {
        Key_Number = 0;
        Set_Speed -= 0.1;
        if(Set_Speed > MOTOR_SPEED_MAX)
            Set_Speed = MOTOR_SPEED_MAX;
        else if(Set_Speed < -MOTOR_SPEED_MAX)
            Set_Speed = -MOTOR_SPEED_MAX;
        icarStr.SpeedSet = Set_Speed;
    }
}
//=============================电机，舵机控制==================================//

void MotorStr_Init(void)
{
    //电机模型初始化,编码器线数，电机减速比，轮子直径，编码器值，都是用来算车速的
    motorStr.EncoderLine = 512.0f;                          //编码器线数=光栅数16*4
    motorStr.ReductionRatio = 2.7f;                         //电机减速比
    motorStr.EncoderValue = 0;                              //编码器值
    motorStr.DiameterWheel = 0.25f;//6.8cm除以4倍             //轮子直径:m
    motorStr.CloseLoop = 1;                                 //闭环控制使能
    ATOM_PWM_InitConfig(ATOMPWM0, 0, MOTOR_Frequenty);      //电机pwm初始化
    PIN_InitConfig(Motor1_Dir, PIN_MODE_OUTPUT, 1);

}
//默认闭环模式
/**
* @brief        电机输出PWM设置
* @param        pwm：-2000~2000
**/
void MOTOR_SetPwmValue(signed int pwm)//了解电机需要几路pwm，及控制电机方向的方式
{
//    pwm = -pwm;
    if(pwm>=0)
    {
        PIN_Write (Motor1_Dir, 1);
        if(pwm>MOTOR_PWM_MAX)
            pwm =MOTOR_PWM_MAX;

        ATOM_PWM_SetDuty(ATOMPWM0, pwm, MOTOR_Frequenty);
    }
    else if(pwm<0)
    {
        PIN_Write (Motor1_Dir, 0);
        if(pwm<MOTOR_PWM_MIN)
            pwm=MOTOR_PWM_MIN;

        pwm = -pwm;
        ATOM_PWM_SetDuty(ATOMPWM0, pwm, MOTOR_Frequenty);
    }
    Data = pwm;
}
/**
* @brief        电机闭环速控
* @param        speed：速度m/s
**/
void MOTOR_ControlLoop(float speed)
{
    if(speed > MOTOR_SPEED_MAX)
        speed = MOTOR_SPEED_MAX;
    else if(speed < -MOTOR_SPEED_MAX)
        speed = -MOTOR_SPEED_MAX;
    //用之前的电机的尺寸及减速比计算出对应的编码器值
    pidStr.vi_Ref = (float)(speed*MOTOR_CONTROL_CYCLE / motorStr.DiameterWheel / PI * motorStr.EncoderLine * 4.0f * motorStr.ReductionRatio);

    MOTOR_SetPwmValue(PID_MoveCalculate(&pidStr));
}

/**
* @brief        电机控制线程
**/

void MOTOR_Timer(void)
{
    motorStr.Counter++;
    if(motorStr.Counter >= 5)                              //速控:10ms
    {
        ENCODER_RevSample();             //编码器采样
        if(motorStr.CloseLoop)
        {
            MOTOR_ControlLoop(icarStr.SpeedSet);        //闭环速控 m/s
        }
        else//开环百分比控制
        {
            if(icarStr.SpeedSet > 100)
               icarStr.SpeedSet = 100;
            else if(icarStr.SpeedSet < -100)
               icarStr.SpeedSet = -100;
            signed int speedRate = MOTOR_PWM_MAX/100.f*icarStr.SpeedSet; //开环：百分比%

            MOTOR_SetPwmValue(speedRate);       //开环速控
        }
        if(usbStr.connected == 0)
        {
            MOTOR_SetPwmValue(0);
        }
        motorStr.Counter = 0;
    }
}
//==============================智能车相关控制=================================//

/**
* @Description  : 智能车参数初始化
**/
void ICAR_Init(void)
{
    icarStr.Electricity = 0;                    //电量信息
    icarStr.Voltage = 0;                        //电压
    icarStr.SpeedSet = 0.0f;                    //电机目标速度：m/s
    icarStr.SpeedFeedback = 0.0f;               //电机模型实测速度：m/s
    icarStr.SpeedMaxRecords = 0.0f;
    icarStr.ring_enter = 0;
    icarStr.ring_inside = 0;
    icarStr.ring_finish = 0;
    icarStr.cater_enter = 0;
//    icarStr.ServoPwmSet = servoStr.thresholdMiddle;
}

//===========================PID控制==============================//
/**
* @brief        PID参数初始化
**/
void PID_Init(void)
{
    //速度式PID
    pidStr.vi_Ref = 0 ;
    pidStr.vi_FeedBack = 0 ;
    pidStr.vi_PreError = 0 ;
    pidStr.vi_PreDerror = 0 ;
    pidStr.v_Kp = PID_VKP;
    pidStr.v_Ki = PID_VKI;
    pidStr.v_Kd = PID_VKD;
    pidStr.vl_PreU = 0;
}


/**
* @brief        PID速控模型
**/
signed int PID_MoveCalculate(PIDStruct *pp)
{
    float  error,d_error,dd_error;

    error = pp->vi_Ref - pp->vi_FeedBack;
    d_error = error - pp->vi_PreError;
    dd_error = d_error - pp->vi_PreDerror;

    pp->vi_PreError = error;
    pp->vi_PreDerror = d_error;

    if( ( error < VV_DEADLINE ) && ( error > -VV_DEADLINE ) )
    {
        ;
    }
    else
    {
//PID积分包和，必要时候启用，消抖
//              I_error = pp -> v_Ki * error;
//              if(I_error >= (VV_MAX/5))
//              {
//              I_error = VV_MAX/5;
//              }
//              else if(I_error <= (VV_MIN/5))
//              {
//              I_error = VV_MIN/5;
//              }
//        pp->vl_PreU += (pp -> v_Kp * d_error + I_error + pp->v_Kd*dd_error)/3;
                  pp->vl_PreU += (pp -> v_Kp * d_error + pp -> v_Ki * error + pp->v_Kd*dd_error);
    }
    if( pp->vl_PreU >= MOTOR_PWM_MAX )
    {
        pp->vl_PreU = MOTOR_PWM_MAX;
    }
    else if( pp->vl_PreU <= MOTOR_PWM_MIN )
    {
        pp->vl_PreU = MOTOR_PWM_MIN;
    }

    return (pp->vl_PreU);
}
