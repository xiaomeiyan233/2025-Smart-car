#ifndef __LQ_TIM_INPUT_H_
#define __LQ_TIM_INPUT_H_

#define  En_Input  ENC5_InPut_P10_3
#define  En_Dir    ENC5_Dir_P10_1




void Encoder_Init(void);
void ENCODER_RevSample(void);
/*************************************************************************
*  函数名称：void Test_GTM_ATOM_PWM(void)
*  修改时间：2020年3月22日
*  备    注：P20.7作为PWM输出口，P33.12作为TIM输入口，两者短接后，串口P14.0发送到上位机
*  默认频率是125HZ，占空比50%
*************************************************************************/
void LQ_TIM_InputCature(void);



#endif
