#ifndef __SYSTICK_H
#define __SYSTICK_H

#include "stm32f10x.h"

#if 1
#define delay_ms(x) SysTick_Delay_Ms(x)
#define delay_us(x) SysTick_Delay_Us(x)

void SysTick_Delay_Us( __IO uint32_t us );
void SysTick_Delay_Ms( __IO uint32_t ms );

#else 
void delay_config( void );
void delay_ms( vu32 nTime );
void delay_us( vu32 nTime );

#endif 

#endif /* __SYSTICK_H */
