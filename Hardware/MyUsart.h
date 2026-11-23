#ifndef __MYUSART_H
#define __MYUSART_H

void MyUsart_Init(void);
void MyUsart_SendByte(uint8_t Byte);
int fputc(int ch, FILE *f);
#endif
