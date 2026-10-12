#include "stm32f10x.h"                  // Device header
#include "Key.h"
#include "Delay.h"
uint8_t key_row[1]={0xff};//????32????

void KEY_Init(void)
{
	
	GPIO_InitTypeDef GPIO_InitStructure;

 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);//使能PORTA时钟
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3;//PA 0 1 2 3
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP; //设置成推挽输出
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA
	
	GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;//PA 4 5 6 7
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; //设置成上拉输入
 	GPIO_Init(GPIOA, &GPIO_InitStructure);//初始化GPIOA
	
} 
int key_scan(void)
{
	static u8 key_up=1;
	int key_num=0;
	u8 temp=0;								//判断列的数据缓存区
	u8 temp1=0;								//判断行的数据缓存区
	GPIO_ResetBits(GPIOA,GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3);			//拉低行线
	Delay_us(10);
	temp=GPIO_ReadInputData(GPIOA)&0xff;				//读出A的数据寄存器的值
	Delay_us(1);
	if (key_up&&(temp != 0xf0))							//判断键是否按下与是否连按
	{
		//Delay_ms(10);						//消抖
		key_up=0;							//当有键按下时，将连按标志位挂起
		temp = GPIO_ReadInputData(GPIOA)&0xff;			//消抖后重新得到A的值
		if(temp != 0xf0)
		{			
			switch(temp)								//进入键盘按下的相应行线
			{
				case 0xe0:
				{
					
					GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);		//依次从第一行拉低到第四行，读出A的数据，从而判断
					GPIO_ResetBits(GPIOA,GPIO_Pin_0);										//是哪一行的键盘按下，从而给出相应的键值
					Delay_us(10);
					temp1=GPIO_ReadInputData(GPIOA)&0xff;
					if (temp1==0xee)
						key_num=1;
					else
					{
						GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
						GPIO_ResetBits(GPIOA,GPIO_Pin_1);
						Delay_us(1);
						temp1=GPIO_ReadInputData(GPIOA)&0xff;
					
						if (temp1==0xed)
							key_num=4;
						else
						{
							GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
							GPIO_ResetBits(GPIOA,GPIO_Pin_2);
							Delay_us(1);
							temp1=GPIO_ReadInputData(GPIOA)&0xff;
						
							if (temp1==0xeb)
								key_num=7;
							else
								key_num=10;
						}
					}
				}
				break;
				case 0xd0:
				{
					GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
					GPIO_ResetBits(GPIOA,GPIO_Pin_0);
					Delay_us(1);
					temp1=GPIO_ReadInputData(GPIOA)&0xff;
					if (temp1==0xde)
						key_num=2;
					else
					{
							GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
							GPIO_ResetBits(GPIOA,GPIO_Pin_1);
							Delay_us(1);
							temp1=GPIO_ReadInputData(GPIOA)&0xff;
						
							if (temp1==0xdd)
								key_num=5;
							else
							{
								GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
								GPIO_ResetBits(GPIOA,GPIO_Pin_2);
								Delay_us(1);
								temp1=GPIO_ReadInputData(GPIOA)&0xff;
							
								if (temp1==0xdb)
									key_num=8;
								else
									key_num=10;
							}
					}
				}
				break;
				case 0xb0:
				{
					GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
					GPIO_ResetBits(GPIOA,GPIO_Pin_0);
					Delay_us(1);
					temp1=GPIO_ReadInputData(GPIOA)&0xff;
					if (temp1==0xbe)
						key_num=3;
					else
					{
							GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
							GPIO_ResetBits(GPIOA,GPIO_Pin_1);
							Delay_us(1);
							temp1=GPIO_ReadInputData(GPIOA)&0xff;
						
							if (temp1==0xbd)
								key_num=6;
							else
							{
								GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
								GPIO_ResetBits(GPIOA,GPIO_Pin_2);
								Delay_us(1);
								temp1=GPIO_ReadInputData(GPIOA)&0xff;
							
								if (temp1==0xbb)
									key_num=9;
								else
									key_num=10;
							}
					}
				}
				break;
				case 0x70:
				{
					GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
					GPIO_ResetBits(GPIOA,GPIO_Pin_0);
					Delay_us(1);
					temp1=(GPIO_ReadInputData(GPIOA)&0xff);
					if (temp1==0x7e)
						key_num=17;
					else
					{
						GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
						GPIO_ResetBits(GPIOA,GPIO_Pin_1);
						Delay_us(1);
						temp1=GPIO_ReadInputData(GPIOA)&0xff;
					
						if (temp1==0x7d)
							key_num=17;
						else
						{
							GPIO_SetBits(GPIOA,GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_0);
							GPIO_ResetBits(GPIOA,GPIO_Pin_2);
							Delay_us(1);
							temp1=GPIO_ReadInputData(GPIOA)&0xff;
						
							if (temp1==0x7b)
								key_num=17;
							else
								key_num=16;
						}
					}
				}
				break;
				default :return 0;
			}		
		}
	}
	else if(temp == 0xf0)
	{												//当没有键按下后，将连按标志复位
		key_up=1;
	}
	return key_num;	
}
 




