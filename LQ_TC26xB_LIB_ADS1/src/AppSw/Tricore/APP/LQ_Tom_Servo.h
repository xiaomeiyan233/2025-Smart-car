#ifndef __LQ_SERVO_H_
#define __LQ_SERVO_H_


#define TOMSERVO1         IfxGtm_TOM0_0_TOUT32_P33_10_OUT
#define TOMSERVO2         IfxGtm_TOM0_13_TOUT35_P33_13_OUT
#define TOMSERVO_0        IfxGtm_TOM0_1_TOUT31_P33_9_OUT
//新车
//相差280
#define  SERVO_PWM_MAX                  2100                        //舵机方向最大PWM
#define  SERVO_PWM_MIN                  500                         //舵机方向最小PWM
#define  SERVO_PWM_MAX_L                865                        //舵机左向转角最大值PWM为1360   SERVO_SetPwmValue(1360)
#define  SERVO_PWM_MAX_R                1545                        //舵机右向转角最大值PWM为2120   SERVO_SetPwmValue(2120)
#define  SERVO_PWM_MIDDLE               1235                        //舵机中值PWM

//老车
//相差380
//舵机中值pwm：1740，左转减小pwm，右转增大pwm
/*
#define  SERVO_PWM_MAX                  1640                        //舵机方向最大PWM
#define  SERVO_PWM_MIN                  880                         //舵机方向最小PWM
#define  SERVO_PWM_MAX_L                1360                        //舵机左向转角最大值PWM为1360   SERVO_SetPwmValue(1360)
#define  SERVO_PWM_MAX_R                2120                        //舵机右向转角最大值PWM为2120   SERVO_SetPwmValue(2120)
#define  SERVO_PWM_MIDDLE               1745                        //舵机中值PWM
*/
#define  SERVO_Frequency                100                         //舵机的频率为100HZ
//时刻同步SERVO_ANGLE_MAX最大角度与SERVO_PWM_MAX_R，SERVO_PWM_MAX_L最左，最右值之间的对等关系
#define  SERVO_ANGLE_MAX                34.2f                       //舵机转角的最大值

typedef struct
{
    uint16_t thresholdMiddle;                   //舵机中值PWM
    uint16_t thresholdLeft;                     //舵机左向转角最大值PWM
    uint16_t thresholdRight;                    //舵机右向转角最大值PWM
}ServoStruct;

extern ServoStruct servoStr;

void LQ_Tom_Servo_2chPWM(void);
void SERVO_PWMInit(void);
void SERVO_SetPwmValue(unsigned int pwm);
void SERVO_SetPwmValueCorrect(unsigned int pwm);
void SERVO_AngleControl(float angle);

#endif
