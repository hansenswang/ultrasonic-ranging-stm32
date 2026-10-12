#include "stm32f10x.h"                  // Device header
#include "OLED.h"
#include "HCSR04.h"
#include "Delay.h"
#include  "dht11.h"
#include "Timer.h"
#include "Buzzer.h"
#include "Key.h"
#include "TM1637.h"

extern unsigned int rec_data[4];
int num[2];
int count=0;
float T0;
float T1;
uint16_t T2;
int alarm_num=10;
int main(void)
{
	OLED_Init();
	HCSR04_Init();
	KEY_Init();
	TM1637_Init();
	//u8 flag_s = 0;
  NVIC_PriorityGroupConfig( NVIC_PriorityGroup_2 );
	int key_val = 0;
	
	OLED_ShowCHinese(0, 0, 0);							//温
	OLED_ShowCHinese(0, 16, 1); 						//度
	OLED_ShowCHinese(0, 32, 4);							//：
	OLED_ShowCHinese(0, 60, 5);							//.
	OLED_ShowCHinese(0, 90, 6);							//℃
	
	
	OLED_ShowCHinese(4, 0, 2);							//湿
	OLED_ShowCHinese(4, 16, 3); 						//度
	OLED_ShowCHinese(4, 32, 4);							//：
	OLED_ShowCHinese(4, 60, 5);							//.
	OLED_ShowCHinese(4, 90, 7);							//%
	
	//OLED_ShowString(4, 1, "distance:");

	
	while (1)
	{
		Delay_ms(200);
		DHT11_REC_Data(); //接收温度和湿度的数据
		Buzzer_Init();
		OLED_ShowNum(1,7,rec_data[2],2);//温度zhengzhu
		OLED_ShowNum(1,10,rec_data[3],1);//温度xiaoshu
		OLED_ShowNum(3,7,rec_data[0],2);//湿度
		OLED_ShowNum(3,10,rec_data[1],2);

		
		T0 = HCSR04_GetValue();
		OLED_ShowFNum(2, 9, T0, 4,1);
		Delay_ms(100);
		OLED_ShowString(2,1, "distance:");
		OLED_ShowString(2,15, "cm");
		T1 = HCSR04compensate_GetValue();
		OLED_ShowFNum(4, 6, T1, 4,1);
		OLED_ShowString(4,1, "cmpst:");
		OLED_ShowString(4,12, "cm");
		Delay_ms(100);
		disp_num[2] = tab[10];
		key_val = key_scan();
		if (key_val > 0 && key_val < 11){
			if (key_val == 10){
			key_val=0;
		}
			num[count]=key_val;
			count++;
		}
		
		if (count==1){
				//OLED_ShowNum(2, 1, num[0], 5);
				disp_num[0] = tab[num[0]];
		}
		else if(count==2){
			//	OLED_ShowNum(2, 1, 10*num[0]+num[1], 5);
			disp_num[0] = tab[num[0]];
			disp_num[1] = tab[num[1]];
		}//在数码管上显示输入数值
		
//		if(count==2){
//			alarm_num=10*num[0]+num[1];
//			OLED_ShowNum(2, 7, alarm_num, 5);
//			disp_num[3] = tab[10];
//			}		//辅助代码（输入最大为两位）
		
		
			if (key_val ==16){
		  if (count==1){
				alarm_num=num[0];
				disp_num[0] = tab[0];
				disp_num[1] = tab[num[0]];
				count=0;
			}
			else if(count==2)
			alarm_num=10*num[0]+num[1];
		//	OLED_ShowNum(2, 7, alarm_num, 5);
			disp_num[3] = tab[15];
			}	//确认键
	
			if (key_val ==(17)){
				num[0]=0;
				num[1]=0;
			  disp_num[0] = tab[21];
			  disp_num[1] = tab[21];
				count=0;
			//OLED_ShowNum(2, 1, num[0], 5);
				disp_num[3] = tab[21];
				alarm_num=10;
			}//清零
	  
		if(T0 < alarm_num)
	{
		Buzzer_ON();
	}
	else if(T0 >= alarm_num)
	{
					Buzzer_OFF();
	}
		 //Delay_ms( 1000 );
     
       TM1637_Display_INC();
       TM1637_SetBrightness( 3 );					//设置亮度等级 0---7
	}
}	
	
