#include "bsp_SysTick.h"

// 这个 SysTick_Config()库函数 在 core_cm3.h中

//下面两种延时方法都可以成功驱动 TM1637，唯一区别就是延时精度的差异


#if 1					//方法一  使用这种方法延时，在主函数中测试1us延时，用示波器测试实际延时为2.44us

// couter 减1的时间 等于 1/systick_clk
// 当counter 从 reload 的值减小到0的时候，为一个循环，如果开启了中断则执行中断服务程序，
// 同时 CTRL 的 countflag 位会置1
// 这一个循环的时间为 reload * (1/systick_clk)

/*
 * t : 定时时间
 * Ticks : 多少个时钟周期产生一次中断
 * f : 时钟频率 72000000
 * t = Ticks * 1/f = (72000000/1000000) * (1/72000000) = 1us
 */

void SysTick_Delay_Us( __IO uint32_t us )
{
    uint32_t i;
    SysTick_Config( SystemCoreClock / 1000000 );

    for( i = 0; i < us; i++ )
    {
        // 当计数器的值减小到0的时候，CRTL寄存器的位16会置1
        while( !( ( SysTick->CTRL ) & ( 1 << 16 ) ) );
    }
    // 关闭SysTick定时器
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}

void SysTick_Delay_Ms( __IO uint32_t ms )
{
    uint32_t i;
    SysTick_Config( SystemCoreClock / 1000 );

    for( i = 0; i < ms; i++ )
    {
        // 当计数器的值减小到0的时候，CRTL寄存器的位16会置1
        // 当置1时，读取该位会清0
        while( !( ( SysTick->CTRL ) & ( 1 << 16 ) ) );
    }
    // 关闭SysTick定时器
    SysTick->CTRL &= ~ SysTick_CTRL_ENABLE_Msk;
}

#else         //方法二  使用这种方法延时，在主函数中测试1us延时，用示波器测试实际延时为1.12us


void delay_config( void )
{
    SysTick->CTRL |= SysTick_CTRL_CLKSOURCE_Msk;     //时钟源为系统时钟 72MHz
    //用示波器测试 当重载值设置为 71时 1us的延时用示波器测试为1.67us
    //根据实际测试 当重载值设置为 50时，1us延时实际测试为 1.12us
    SysTick->LOAD = 51 - 1;                          //重载值为72-1，每1us溢出一次
}

void delay_ms( vu32 nTime )
{
    nTime *= 1000;
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;        //使能SysTick，开始计数
    while( nTime-- )
    {
        while( ( SysTick->CTRL & 0X010000 ) == 0 );  //等待COUNTFLAG标志位置1
    }
    SysTick->CTRL &= ( ~SysTick_CTRL_ENABLE_Msk );   //失能SysTick，停止计数
}

void delay_us( vu32 nTime )
{
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    while( nTime-- )
    {
        while( ( SysTick->CTRL & 0X010000 ) == 0 );
    }
    SysTick->CTRL &= ( ~SysTick_CTRL_ENABLE_Msk );
}


#endif 

/*********************************************END OF FILE**********************/
