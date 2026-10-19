#include "stm32f10x.h"                  // Device header
#include "Delay.h"
#include "OLED.h"
#include "COUNTSENSOR.h"
#include "Encoder.h"

int16_t Num;			//定义待被旋转编码器调节的变量

int main(void)
{
	/*模块初始化*/
	OLED_Init();			//OLED初始化：PB6(SCL) / PB7(SDA)，软件 I2C
	COUNTSENSOR_Init();		//对射式红外传感器：PB14 / EXTI14（上升沿触发）
	Encoder_Init();			//旋转编码器：PB0(A相)/PB1(B相) / EXTI0、EXTI1（下降沿触发）

	/*显示静态字符串*/
	OLED_ShowString(1, 1, "Sensor:");		//第1行：传感器遮挡次数
	OLED_ShowString(2, 1, "Num:");			//第2行：旋转编码器累加值

	while (1)
	{
		/*对射式红外传感器计次：中断里 +1，这里只负责显示*/
		OLED_ShowNum(1, 8, CountSensor_Get(), 5);

		/*旋转编码器：取出「上次调用之后」的增量值，再累加到 Num 上*/
		Num += Encoder_Get();
		OLED_ShowSignedNum(2, 5, Num, 5);
	}
}
