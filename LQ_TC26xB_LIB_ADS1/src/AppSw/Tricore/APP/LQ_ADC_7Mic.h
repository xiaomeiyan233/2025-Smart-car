#ifndef __LQ_7mic_H_
#define __LQ_7mic_H_

typedef struct
{
   float Max_Voltage;
   float Min_Voltage;
   float Now_Voltage;
   float Differ_Value;
   float Max_Min_Range;
   char  Warning_sign;
   int Percentage_voltage;
}Voltage_Detection;

extern Voltage_Detection Measuring_Voltage;

void ADC_Voltage_Detection(void);
void ADC_Detection_Init(void);
void Test_ADC(void);
void Test_ADC_7mic(void);
void Test_ADC_TFT(void);

#endif
