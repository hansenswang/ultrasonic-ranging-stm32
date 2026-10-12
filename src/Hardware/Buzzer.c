#include "stm32f10x.h"                  // Device header
extern uint16_t T0;
void Buzzer_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
}

void Buzzer_ON(void)
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}

void Buzzer_OFF(void)
{
	GPIO_SetBits(GPIOB, GPIO_Pin_13);
}

void Buzzer_Turn(void)
{
	if (GPIO_ReadOutputDataBit(GPIOB, GPIO_Pin_13) == 0)
	{
		GPIO_SetBits(GPIOB, GPIO_Pin_13);
	}
	else
	{
		GPIO_ResetBits(GPIOB, GPIO_Pin_13);
	}
}
void alarm(int alarm_num)
{

	
	if(T0 < alarm_num)
	{
		Buzzer_ON();
	}
	else if(T0 >= alarm_num)
	{
					Buzzer_OFF();
	}
}

