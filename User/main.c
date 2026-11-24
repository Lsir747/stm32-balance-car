#include <stdio.h>
#include "stm32f10x.h"                  // Device header
#include "MY_MPU6050.h"
#include "MPU6050_Reg.h"
#include  "MyUsart.h"


extern float AG_AllData[6];
extern uint8_t WaitOverTimeFlag;
extern uint8_t MPU6050_ID;

int main(void)
{
	Recover();

	MY_MPU6050_Init();
	MyUsart_Init();
	
	while(1)
	{

		MPU6050_ReadData(I2C2,MPU6050_WHO_AM_I,1);

		
		MPU6050_ReadData(I2C2,MPU6050_ACCEL_XOUT_H,14);
		Merge();

		
		printf("ID:%X    Ax=%.2f    Ay=%.2f    Az=%.2f    Gx=%.2f    Gy=%.2f    Gz=%.2f\r\n",
			MPU6050_ID,AG_AllData[0] ,AG_AllData[1] ,AG_AllData[2], AG_AllData[3],AG_AllData[4],AG_AllData[5]);
	}
}
