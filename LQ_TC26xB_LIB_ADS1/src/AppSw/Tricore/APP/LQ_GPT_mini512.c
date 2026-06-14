#include <LQ_GPIO_LED.h>
#include <LQ_GPT12_ENC.h>
#include <LQ_STM.h>
#include <LQ_UART.h>
#include <stdint.h>
#include <stdio.h>

volatile        int encValue1  = 0;
volatile		int encValue2  = 0;
volatile		int encValue3  = 0;
volatile		int encValue4  = 0;
volatile		int encValue5  = 0;

void LQ_GPT_4mini512(void)
{
  ENC_InitConfig(ENC2_InPut_P33_7, ENC2_Dir_P33_6);
  //ENC_InitConfig(ENC3_InPut_P02_6, ENC3_Dir_P02_7);//摄像头冲突，不建议用
  ENC_InitConfig(ENC4_InPut_P02_8, ENC4_Dir_P33_5);
  ENC_InitConfig(ENC5_InPut_P10_3, ENC5_Dir_P10_1);
  ENC_InitConfig(ENC6_InPut_P20_3, ENC6_Dir_P20_0);
  while(1)
  {
  }
}
