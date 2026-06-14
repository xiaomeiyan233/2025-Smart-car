#include <IfxCpu.h>
#include <IfxScuWdt.h>
#include <LQ_CCU6.h>
#include <LQ_GPIO_KEY.h>
#include <LQ_SOFTI2C.h>
#include <Main.h>
#include <Platform_Types.h>
#include <stdint.h>
#include <stdio.h>
#include <LQ_ADC.h>

// 定时器 5ms和50ms标志位
volatile uint8 cpu1Flage5ms = 0;
volatile uint8 cpu1Flage50ms = 0;

// 期望速度
volatile sint16 targetSpeed = 10;

// 避障标志位
volatile uint8 evadibleFlage = 0;

int core1_main (void)
{
    // 开启CPU总中断
    IfxCpu_enableInterrupts();

    // 关闭看门狗
    IfxScuWdt_disableCpuWatchdog (IfxScuWdt_getCpuWatchdogPassword ());

    // 等待CPU0 初始化完成
    while(!IfxCpu_acquireMutex(&mutexCpu0InitIsOk));

    // 所有含有中断的测试都默认在CPU0中执行，如果需要用CPU1请参考龙邱B站视频。
    // 程序配套视频地址：https://space.bilibili.com/95313236
    while(1)//主循环
    {

    }
}
