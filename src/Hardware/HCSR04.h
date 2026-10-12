#ifndef __HCSR04_H
#define __HCSR04_H

#define Trig_Port 		GPIOA
#define Trig_Pin 		GPIO_Pin_8
#define Trig_RCC		RCC_APB2Periph_GPIOA

#define Echo_Port 		GPIOA
#define Echo_Pin 		GPIO_Pin_9
#define Echo_RCC		RCC_APB2Periph_GPIOA

void HCSR04_Init(void);
float HCSR04_GetValue(void);
float HCSR04compensate_GetValue(void);

#endif 
