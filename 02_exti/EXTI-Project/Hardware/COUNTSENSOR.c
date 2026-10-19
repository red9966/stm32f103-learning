#include "stm32f10x.h"                  // Device header
uint16_t CountSensor_Count;	
void COUNTSENSOR_Init(void)
{
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_14;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
	
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource14);
	EXTI_InitTypeDef Exit_InitStructure;
	Exit_InitStructure.EXTI_Line=EXTI_Line14;
	Exit_InitStructure.EXTI_LineCmd=ENABLE;
	Exit_InitStructure.EXTI_Mode=EXTI_Mode_Interrupt;
	Exit_InitStructure.EXTI_Trigger=EXTI_Trigger_Rising;
	EXTI_Init(&Exit_InitStructure);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);	
	NVIC_InitTypeDef NVIC_InitStructure;						//定义结构体变量
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;		//选择配置NVIC的EXTI15_10线
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;				//指定NVIC线路使能
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;	//指定NVIC线路的抢占优先级为1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;			//指定NVIC线路的响应优先级为1
	NVIC_Init(&NVIC_InitStructure);			

}
uint16_t CountSensor_Get(void)
{
	return CountSensor_Count;
}

void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line14)==SET)
	{
	CountSensor_Count ++;	
	
	}
	EXTI_ClearITPendingBit( EXTI_Line14);
}
