#include "stm32f10x.h"                  // Device header
#include "MPU6050_Reg.h"
#include "Delay.h"
#include <math.h>
#define  a	0.96	//互补滤波陀螺仪权重
#define  dt 0.005	//单位s，积分时长5ms		
			//I2C耗时：14 字节数据 + 地址 + 寄存器地址 ≈ 17 字节： 17 × 9 ÷ 400000 = 0.38 ms
			//USART阻塞式打印耗时：字符数 × 10 位 ÷ 波特率
			//单轮积分时长0.4~1.3 ms
uint8_t WaitOverTimeFlag = 0;		//超时标志位，置1表示WairEven函数超时
uint8_t MPU6050_ID;					//存放芯片ID
float AG_AllData[6];				//MPU6050六轴数据
uint8_t BufferData[14];					//MPU6050六轴数据缓冲区
float SampleData_X,SampleData_Y,SampleData_Z;		//三轴陀螺仪采样误差
float X_Angle;
volatile uint8_t SampleFlag = 0;

void TIM_SetFlag(void)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2,ENABLE);
	
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInitStruct.TIM_Period = 5000 - 1;
	TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1;
	TIM_TimeBaseInitStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM2,&TIM_TimeBaseInitStruct);
	
	TIM_ClearFlag(TIM2,TIM_FLAG_Update);
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
	
	NVIC_Init(&NVIC_InitStruct);
	
	TIM_Cmd(TIM2,ENABLE);
	
}

void TIM2_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update)==SET)
	{
		TIM_ClearITPendingBit(TIM2,TIM_IT_Update);
		SampleFlag = 1;
	}
		
}

float Attitude_Estimation(void)
{
	
	
	X_Angle = a * ( X_Angle + AG_AllData[4] * dt ) + ( 1 - a )*( atan2 ( AG_AllData[0] , AG_AllData[2] ) * 57.29578 );
	return X_Angle;
}



void MPU6050_ZOC(void)
{
	AG_AllData[3] -= SampleData_X;
	AG_AllData[4] -= SampleData_Y;
	AG_AllData[5] -= SampleData_Z; 
}
void Merge(void)
{
	/*		将六轴原始数据拼接并换算成物理量		*/
	AG_AllData[0] = (int16_t)(((uint16_t)BufferData[0]<<8)|BufferData[1])/8192.0;
	AG_AllData[1] = (int16_t)(((uint16_t)BufferData[2]<<8)|BufferData[3])/8192.0;
	AG_AllData[2] = (int16_t)(((uint16_t)BufferData[4]<<8)|BufferData[5])/8192.0;
	AG_AllData[3] = (int16_t)(((uint16_t)BufferData[8]<<8)|BufferData[9])/65.5;
	AG_AllData[4] = (int16_t)(((uint16_t)BufferData[10]<<8)|BufferData[11])/65.5; 
	AG_AllData[5] = (int16_t)(((uint16_t)BufferData[12]<<8)|BufferData[13])/65.5;  
	                          
}
/*		超时等待函数		*/
void WaitEven(I2C_TypeDef *I2Cx,uint32_t Event)
{
	uint16_t Timeout =10000;
	while(I2C_CheckEvent(I2Cx,Event) != SUCCESS)
	{
		Timeout--;
		if( Timeout == 0 )
		{
			WaitOverTimeFlag = 1;	
			break;
		}
	}
}

/*		指定I2C外设，MPU6050指定寄存器写入一字节数据		*/
void MPU6050_WriteReg(I2C_TypeDef *I2Cx,uint8_t Reg_Address,uint8_t Data)
{
	I2C_GenerateSTART(I2Cx,ENABLE);
	WaitEven(I2Cx,I2C_EVENT_MASTER_MODE_SELECT);
	
	I2C_Send7bitAddress(I2Cx,MPU6050_Address,I2C_Direction_Transmitter);
	WaitEven(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
	
	I2C_SendData(I2Cx,Reg_Address);
	WaitEven(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	
	I2C_SendData(I2Cx,Data);
	WaitEven(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	
	I2C_GenerateSTOP(I2Cx,ENABLE);
}

/*		stm32I2C外设及MPU6050模块初始化		*/
void MY_MPU6050_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;							//复用开漏输出
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	I2C_InitTypeDef I2C_InitStruct;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;								//应答位开
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 400000;									//I2C通信速度40KHZ
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_Init(I2C2,&I2C_InitStruct);
	
	I2C_Cmd(I2C2,ENABLE);
	
/*		配置MPU6050		*/
	MPU6050_WriteReg(I2C2,MPU6050_PWR_MGMT_1,0x01);							//
	MPU6050_WriteReg(I2C2,MPU6050_PWR_MGMT_2,0x00);							//
	MPU6050_WriteReg(I2C2,MPU6050_SMPLRT_DIV,0x00);							//陀螺仪采样频率改为1kHz
	MPU6050_WriteReg(I2C2,MPU6050_CONFIG,0x03);								//带宽42Hz，延迟4.9ms
	MPU6050_WriteReg(I2C2,MPU6050_GYRO_CONFIG,0x08);						//角速度量程改为-+500°/s
	MPU6050_WriteReg(I2C2,MPU6050_ACCEL_CONFIG,0x08);						//加速度量程改为 -+4g
}

//从MPU6050指定寄存器读取Count次数据存入缓冲区数组中
void MPU6050_ReadData(I2C_TypeDef *I2Cx,uint8_t Reg_Address,uint8_t Count)
{
	uint8_t i ;

	I2C_GenerateSTART(I2Cx,ENABLE);
	WaitEven(I2Cx,I2C_EVENT_MASTER_MODE_SELECT);
	
	
	I2C_Send7bitAddress(I2Cx,MPU6050_Address,I2C_Direction_Transmitter);
	WaitEven(I2Cx,I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED);
	
	
	I2C_SendData(I2Cx,Reg_Address);
	WaitEven(I2Cx,I2C_EVENT_MASTER_BYTE_TRANSMITTED);
	
	
	I2C_GenerateSTART(I2Cx,ENABLE);
	WaitEven(I2Cx,I2C_EVENT_MASTER_MODE_SELECT);
	
	
	I2C_Send7bitAddress(I2Cx,MPU6050_Address,I2C_Direction_Receiver);
	WaitEven(I2Cx,I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED);
	
	if(Count == 1)								//单字节读取视为读ID，数据存入ID变量中
	{
		I2C_AcknowledgeConfig(I2Cx,DISABLE);
		I2C_GenerateSTOP(I2Cx,ENABLE);
		
		WaitEven(I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED);
		MPU6050_ID = I2C_ReceiveData(I2Cx);
	}else										//多字节读取视为读六轴加温度数据，存入缓冲区数组中
	{
		for( i=0;i<Count;i++)
		{
			if(i == (Count - 1))
			{
				I2C_AcknowledgeConfig(I2Cx,DISABLE);
				I2C_GenerateSTOP(I2Cx,ENABLE);
			}
			WaitEven(I2Cx,I2C_EVENT_MASTER_BYTE_RECEIVED);
		
			BufferData[i] = I2C_ReceiveData(I2Cx);
		}
	}
	
	I2C_AcknowledgeConfig(I2Cx,ENABLE);
	
	
}
/*		零偏采样校准		*/
void  MPU6050_Sample_error(void)
{
	uint16_t i;
	
	for(i = 0; i<1000 ;i++)
	{
		MPU6050_ReadData(I2C2,MPU6050_ACCEL_XOUT_H,14);
		Merge();
		SampleData_X += AG_AllData[3];  SampleData_Y += AG_AllData[4];  SampleData_Z+= AG_AllData[5];
		
	}
	SampleData_X /= 1000.0f;  SampleData_Y /= 1000.0f;  SampleData_Z /= 1000.0f;
	
}

void Recover(void)
{
	//开启外设时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_I2C2,ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);
	
	//I2C外设失能
	I2C_Cmd(I2C2,DISABLE);
	
	//I2C外设复位重置
	I2C_SoftwareResetCmd(I2C2,ENABLE);
	I2C_SoftwareResetCmd(I2C2,DISABLE);
	
	
	//将SCLSDA引脚切换到普通开漏输出以手动输出SCL时钟
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 |GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//切换到普通开漏输出时，ODR寄存器本身置0，默认输出引脚为低电平
	//此步骤主要为了防止主机一直拉低SDA而无法给从机发送非应答
	//从机会误以为主机还要数据而继续操控SDA
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
	
	//给足9个SCL时钟，确保从机发完当前帧未发送的数据
	for(uint8_t i=0;i<9;i++)
	{
		GPIO_ResetBits(GPIOB,GPIO_Pin_10);
		Delay_us(10);
		GPIO_SetBits(GPIOB,GPIO_Pin_10);
		Delay_us(10);
	}
	
	//给从机发送STOP信号
	GPIO_ResetBits(GPIOB,GPIO_Pin_11);
	Delay_us(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_10);
	Delay_us(10);
	GPIO_SetBits(GPIOB,GPIO_Pin_11);
	
	//救机完成后，重新配置GPIO引脚为复用开漏输出
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_OD;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 |GPIO_Pin_11;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB,&GPIO_InitStruct);
	
	//重新配置I2C外设
	I2C_InitTypeDef I2C_InitStruct;
	I2C_InitStruct.I2C_Ack = I2C_Ack_Enable;
	I2C_InitStruct.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
	I2C_InitStruct.I2C_ClockSpeed = 400000;
	I2C_InitStruct.I2C_DutyCycle = I2C_DutyCycle_2;
	I2C_InitStruct.I2C_Mode = I2C_Mode_I2C;
	I2C_InitStruct.I2C_OwnAddress1 = 0x00;
	I2C_Init(I2C2,&I2C_InitStruct);
	
	//I2C外设使能
	I2C_Cmd(I2C2,ENABLE);
	
}
