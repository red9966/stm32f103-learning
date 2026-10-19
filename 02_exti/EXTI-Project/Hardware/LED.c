#include "stm32f10x.h"                  // Device header
void LED_Init(void)
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_0|GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
	GPIO_Init(GPIOA,&GPIO_InitStructure);
	
}
void LED0_RETURN(void)
{	if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_0)==0)
	{
	GPIO_SetBits(GPIOA, GPIO_Pin_0);
		
	}
	
	else{
	GPIO_ResetBits(GPIOA, GPIO_Pin_0);
	}

}
void LED1_RETURN(void)
{	if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_3)==0)
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_3);//该位置1熄灭
		
	}
	
	else{
		GPIO_ResetBits(GPIOA, GPIO_Pin_3);//该位置0点亮
	}

}



void LED_0_ON(void)
{

GPIO_ResetBits(GPIOA,GPIO_Pin_0);

}


void LED_0_OFF(void)
{

GPIO_SetBits(GPIOA,GPIO_Pin_0);

}

void LED_1_ON(void)
{

GPIO_ResetBits(GPIOA,GPIO_Pin_1);

}


void LED_1_OFF(void)
{

GPIO_SetBits(GPIOA,GPIO_Pin_1);

}
