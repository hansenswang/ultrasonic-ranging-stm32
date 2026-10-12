#include "stm32f10x.h"                  // Device header
#include "HCSR04.h"
#include "Timer.h"
#include "Delay.h"
#include  "dht11.h"
extern unsigned int rec_data[4];
uint16_t N;

void HCSR04_Init()
{
	RCC_APB2PeriphClockCmd(Trig_RCC, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Pin = Trig_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Trig_Port, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPD;
	GPIO_InitStruct.GPIO_Pin = Echo_Pin;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(Echo_Port, &GPIO_InitStruct);
	
	GPIO_ResetBits(Trig_Port, Trig_Pin);
	
}

void HCSR04_Start()
{
	GPIO_SetBits(Trig_Port, Trig_Pin);
	Delay_us(45);
	GPIO_ResetBits(Trig_Port, Trig_Pin);
	Timer_Init();
}

float HCSR04_GetValue()
{
	HCSR04_Start();
	Delay_ms(100);
	return ((N * 0.00001) * 34000) / 2;
}
float HCSR04compensate_GetValue()
{
	HCSR04_Start();
	Delay_ms(100);
	return ((N * 0.00001) * (331.4+0.606*rec_data[2]+0.0124*rec_data[0])*100) / 2;
}

