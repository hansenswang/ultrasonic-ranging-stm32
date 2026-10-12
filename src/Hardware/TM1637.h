#ifndef __TM1637_H
#define __TM1637_H

#include "sys.h"
extern unsigned char tab[];
extern unsigned char disp_num[];

//使用 位带 操作
#define TM1637_CLK           PBout(6)
#define TM1637_DIO           PBout(7)
#define TM1637_READ_DIO      PBin(7)

# if 0
//使用库函数操作
#define TM1637_CLK_Low()     GPIO_ResetBits(TM1637_CLK_GPIO_PORT,TM1637_CLK_GPIO_PIN)
#define TM1637_CLK_High()    GPIO_SetBits(TM1637_CLK_GPIO_PORT,TM1637_CLK_GPIO_PIN)

#define TM1637_DIO_Low()     GPIO_ResetBits(TM1637_DIO_GPIO_PORT,TM1637_DIO_GPIO_PIN)
#define TM1637_DIO_High()    GPIO_SetBits(TM1637_DIO_GPIO_PORT,TM1637_DIO_GPIO_PIN)

#define TM1637_DIO_READ()    GPIO_ReadInputDataBit(TM1637_DIO_GPIO_PORT, TM1637_DIO_GPIO_PIN)	/* 读SDA口线状态 */
#endif

//IO方向设置		   								0011输出模式   1000上下拉输入模式
#define TM1637_DIO_IN()      {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)8<<28;}
#define TM1637_DIO_OUT()     {GPIOB->CRL&=0X0FFFFFFF;GPIOB->CRL|=(u32)3<<28;}


/* 定义IIC连接的GPIO端口, 用户只需要修改下面的代码即可改变控制的LED引脚 */
#define TM1637_CLK_GPIO_PORT    	GPIOB		                /* GPIO端口 */
#define TM1637_CLK_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define TM1637_CLK_GPIO_PIN			  GPIO_Pin_6

#define TM1637_DIO_GPIO_PORT    	GPIOB			              /* GPIO端口 */
#define TM1637_DIO_GPIO_CLK 	    RCC_APB2Periph_GPIOB		/* GPIO端口时钟 */
#define TM1637_DIO_GPIO_PIN		    GPIO_Pin_7



void TM1637_Init( void );
void TM1637_Start( void );
void TM1637_Ack( void );
void TM1637_Stop( void );
void TM1637_WriteByte( unsigned char oneByte );
unsigned char TM1637_ScanKey( void );
void TM1637_NixieTubeDisplay( void );

void TM1637_SetBrightness( unsigned char level );
void TM1637_Display_INC( void );
void TM1637_Display_NoINC( unsigned char add, unsigned char value );
unsigned char TM1637_KeyProcess( void );

#endif
