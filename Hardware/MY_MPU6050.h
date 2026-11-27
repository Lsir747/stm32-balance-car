#ifndef __MY_MPU6050_h
#define __MY_MPU6050_h

extern volatile uint8_t SampleFlag ;

void MY_MPU6050_Init(void);
void MPU6050_WriteReg(I2C_TypeDef *I2Cx,uint8_t Reg_Address,uint8_t Data);
void MPU6050_ReadData(I2C_TypeDef *I2Cx,uint8_t Reg_Address,uint8_t Count);
void WaitEven(I2C_TypeDef *I2Cx,uint8_t Event);
void Recover(void);
void Merge(void);
void MPU6050_ZOC(void);
void  MPU6050_Sample_error(void);
float Attitude_Estimation(void);
void TIM_SetFlag(void);

#endif
