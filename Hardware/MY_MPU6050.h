#ifndef __MY_MPU6050_h
#define __MY_MPU6050_h

void MY_MPU6050_Init(void);
void MPU6050_WriteReg(I2C_TypeDef *I2Cx,uint8_t Reg_Address,uint8_t Data);
void MPU6050_ReadData(I2C_TypeDef *I2Cx,uint8_t Reg_Address,uint8_t Count);
void WaitEven(I2C_TypeDef *I2Cx,uint8_t Event);
void Recover(void);
void Merge(void);


#endif
