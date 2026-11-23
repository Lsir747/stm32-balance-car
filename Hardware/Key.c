#include "stm32f10x.h"                  // Device header
#include "Delay.h"

void Key_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStructure);
}
uint8_t Key_GetNum(void)
{
	uint8_t KeyNum = 0;
	
	/*********pin_1按键状态读取*************************************/
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0 )//按键按下读取到低电平
	{
		Delay_ms(20);//消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1) == 0 )//一直处于按下状态就一直重复循环消抖直到松手
		Delay_ms(20);//消抖
		KeyNum = 1;
	}
	
	/*********pin_11按键状态读取*************************************/
	if (GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0 )//按键按下读取到低电平
	{
		Delay_ms(20);//消抖
		while(GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_11) == 0 )//一直处于按下状态就一直重复循环消抖直到松手
		Delay_ms(20);//消抖
		KeyNum = 2;
	}
	
	return KeyNum;
}
