#include <LQ_ADC.h>
#include <LQ_GPIO_LED.h>
#include <LQ_STM.h>
#include <LQ_UART.h>
#include <stdio.h>
#include "LQ_ADC_7Mic.h"
#include <LQ_TFT18.h>

Voltage_Detection Measuring_Voltage;
char txt[32];
void ADC_Detection_Init(void)
{
    Measuring_Voltage.Max_Voltage = 1.156; //12.52V
    Measuring_Voltage.Min_Voltage = 1.040; //11.51V  1.060
    Measuring_Voltage.Now_Voltage = 0;
    Measuring_Voltage.Differ_Value = 0;
    Measuring_Voltage.Max_Min_Range = Measuring_Voltage.Max_Voltage - Measuring_Voltage.Min_Voltage;
    Measuring_Voltage.Warning_sign = 0;
    Measuring_Voltage.Percentage_voltage = 0;

    ADC_InitConfig(ADC0, 80000);
    ADC_InitConfig(ADC1, 80000);
    ADC_InitConfig(ADC2, 80000);
    ADC_InitConfig(ADC3, 80000);
    ADC_InitConfig(ADC4, 80000);
    ADC_InitConfig(ADC5, 80000);
    ADC_InitConfig(ADC6, 80000);
    ADC_InitConfig(ADC7, 80000);
}
void ADC_Voltage_Detection(void)
{
    short vbat;
    float Test_Voltage;
    static int Sum_Data = 0;
    static int Count1 = 0;
    vbat = ADC_Read(ADC7);
    Sum_Data+=vbat;
    Count1++;
    if(Count1>=50)
    {
        vbat = Sum_Data/50;
        Count1 = 0;
        Sum_Data = 0;
        Measuring_Voltage.Now_Voltage = vbat/4095.0*3.3;

        sprintf(txt, "voltage:%.3f", Measuring_Voltage.Now_Voltage);               //电机最终设置的pwm值，可设置的最大值为10000
        TFTSPI_P8X16Str(1,7,txt,u16WHITE,u16BLACK);

        Test_Voltage = vbat/4095.0*3.3;
        Measuring_Voltage.Percentage_voltage = (int)((Measuring_Voltage.Now_Voltage-Measuring_Voltage.Min_Voltage)*100.0/Measuring_Voltage.Max_Min_Range);
        if(Measuring_Voltage.Percentage_voltage>1 && Measuring_Voltage.Percentage_voltage <= 150)
        {
            Measuring_Voltage.Warning_sign = 0;
        }
        else
        {
            Measuring_Voltage.Warning_sign = 1;
        }
    }
    if(Measuring_Voltage.Now_Voltage <= 1.050 && Measuring_Voltage.Now_Voltage != 0)
    {
        //GPIO_BuzzerEnable(BuzzerWarnning);
    }


}
