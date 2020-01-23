#ifndef _HY1602_H_
#define _HY1602_H_

#ifndef uchar
#define uchar unsigned char
#endif
#ifndef uint
#define uint unsigned int
#endif

extern void A1602_GPIO_Configuration(void);
extern void LCD_Init(void);
extern void LCD_write_str(uchar x, uchar y, char str[]);

#endif
