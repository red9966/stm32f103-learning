#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "LED.h"
#include "KEY.h"
uint8_t KeyNum;
int main(void)
{	
	LED_Init();	
	KEY_Init() ;
		

	
	while(1)
	{
				KeyNum = K_GETNUM();		//获取按键键码
		
		if (KeyNum == 1)			//按键1按下
		{
			LED0_RETURN();			//LED1翻转
		}
		
		if (KeyNum == 2)			//按键2按下
		{
			LED1_RETURN();			//LED2翻转
		}

		
		
	}





 



}
