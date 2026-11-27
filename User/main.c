#include <stdio.h>
#include "stm32f10x.h"                  // Device header
#include "MY_MPU6050.h"
#include "MPU6050_Reg.h"
#include  "MyUsart.h"

extern float SampleData_X,SampleData_Y,SampleData_Z;
extern float AG_AllData[6];
extern uint8_t WaitOverTimeFlag;
extern uint8_t MPU6050_ID;

int main(void)
{
	float X_Angle;
	uint8_t printf_cnt = 0;
	Recover();

	MY_MPU6050_Init();
	MyUsart_Init();
	MPU6050_Sample_error();
	MPU6050_ReadData(I2C2,MPU6050_WHO_AM_I,1);
	TIM_SetFlag();

	while(1)
	{
		
		if(SampleFlag)
		{
			SampleFlag = 0;
			MPU6050_ReadData(I2C2,MPU6050_ACCEL_XOUT_H,14);
			Merge();
			MPU6050_ZOC();
			X_Angle = Attitude_Estimation();
			if(++printf_cnt>=10)
			{ 
//				printf("ID:%X    Ax=%.2f    Ay=%.2f    Az=%.2f    Gx=%.2f    Gy=%.2f    Gz=%.2f\r\n",
//				MPU6050_ID,AG_AllData[0] ,AG_AllData[1] ,AG_AllData[2], AG_AllData[3],AG_AllData[4],AG_AllData[5]);
				printf("X_Angle:%.2f, Ax=%.2f  Az=%.2f  Gy=%.2f\r\n",X_Angle,AG_AllData[0],AG_AllData[2],AG_AllData[4]);
				printf_cnt = 0;
			}
			
		}
		//降低printf频率
		
		
		
	}
}
