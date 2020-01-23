/**************************************************************************
**文件名称：stm_1602.c  for  STM32
**by zyy  2013-12-02
**************************************************************************/
#include "stm32f10x.h"
#include "1602.h"

#define LCD_RS_1 GPIOE->BSRR=GPIO_Pin_0 //PE.0--(LCD)RS
#define LCD_RS_0 GPIOE->BRR =GPIO_Pin_0
#define LCD_RW_1 GPIOE->BSRR=GPIO_Pin_1 //PE.1--(LCD)RW
#define LCD_RW_0 GPIOE->BRR =GPIO_Pin_1
#define LCD_EN_1 GPIOE->BSRR=GPIO_Pin_2 //PE.2--(LCD)E
#define LCD_EN_0 GPIOE->BRR =GPIO_Pin_2
#define DATAOUT  GPIOD->ODR             //PD[0..7]--(LCD)D0~D7

extern void delay_nms(unsigned long n);
extern void delay_nus(unsigned long n);

void LCD_Writecmd(uchar cmd);
void LCD_Writedata(uchar dat);
void LCD_write_char(uchar row, uchar col, char ch);
void LCD_write_str(uchar x, uchar y, char str[]);
void LCD_clear(void);
void LCD_Init(void);
//延时n us
void delay_nus(unsigned long n);
void delay_nms(unsigned long n);
void A1602_GPIO_Configuration(void);
void LCD_Init(void);

/**************************************************************************
**文件名称：stm_1602.c  for  STM32
**by zyy  2013-12-02
**************************************************************************/



/*********************************************************************
*名    称：LCD_Writecmd()
*功    能：写指令到LCD1602,指令数据占一个字节
*入口参数：cmd:待写入的指令
*出口参数：无
*********************************************************************/
void LCD_Writecmd(uchar cmd)
{
    LCD_RS_0;    //对同一个寄存器的两次写入，中间延时一会
    delay_nus(1);
    LCD_RW_0;
    delay_nus(1);

    LCD_EN_1;
    DATAOUT = cmd;
    delay_nus(300);//必要的延时
    LCD_EN_0;    //下降沿，LCD1602开始工作
}
/*********************************************************************
*名    称：LCD_Writedata()
*功    能：写一字节数据到LCD1602
*入口参数：dat：无符号字节类型，0~255  包括各个ASCII码字符
*出口参数：无
*********************************************************************/
void LCD_Writedata(uchar dat)
{
    LCD_RS_1;
    delay_nus(1);
    LCD_RW_0;
    delay_nus(1);
    LCD_EN_1;  //先拉高
    DATAOUT = dat;
    delay_nus(300); //很重要的延时，经调试，延时300us以上才可以
    LCD_EN_0;       //下降沿，开始写入有效数据
}
/*********************************************************************
*功  能：在指定位置显示一个字符
*入口参数：y:行 0或1        x:列，0~15，ch：待显示的字符
*********************************************************************/
void LCD_write_char(uchar x, uchar y, char ch)
{
    if (y == 0)
    {
        LCD_Writecmd(0x80 + x);
    }
    else
    {
        LCD_Writecmd(0xC0 + x);
    }
    LCD_Writedata(ch);
}
/*********************************************************************
*名    称：LCD_write_str()
*功    能：使LCD1602显示一个字符串
*入口参数：y:行 0或1        x:列，0~15，str[]:待显示的字符串
*出口参数：无
*********************************************************************/
void LCD_write_str(uchar x, uchar y, char str[])
{
    uchar i = 0;
    if (y == 0)
    {
        LCD_Writecmd(0x80 + x);
    }
    else
    {
        LCD_Writecmd(0xC0 + x);
    }
    while (str[i] != '\0')
    {
        LCD_Writedata(str[i]);
        ++i;
    }
}
/******************************************************************/
/*                      清屏函数                                */
/******************************************************************/
void LCD_clear(void)
{
    LCD_Writecmd(0x01);
    delay_nms(5);
}

/*********************************************************************
*名    称：LCD_Init()
*功    能：初始化LCD1602
*入口参数：无
*出口参数：无
*常用命令：0x38:16*2显示，5*&点阵显示字符，8位数据;指令执行时间40us
*          0x0C:开显示，关光标   40us
*          0x08:关显示，关光标   40us
*          0x0D:字符闪烁，关光标，接着设定位置，闪烁周期0.4ms左右
*          0x0F:字符闪烁，开光标
*          0x06:写完数据自动右移光标，普通情形，从左向右显示  40us
*          0x04:写完数据自动左移光标，可以从右向左显示  40us
*          0x01:清除显示内容，即清屏  1.64ms
*          0x02:使光标还回起始位置   1.64ms
*          0x18:屏幕上所有字符同时左移一格，适合滚动显示 40us
*          0x1C:屏幕上所有字符同时右移一格，适合滚动显示 40us
*********************************************************************/
void LCD_Init(void)
{
    delay_nms(100);           //延时20ms
    LCD_Writecmd(0x38);  //16*2显示，5*7点阵，8位数据
    delay_nms(10);
    LCD_Writecmd(0x38);  //16*2显示，5*7点阵，8位数据
    delay_nms(10);
    LCD_Writecmd(0x38);  //16*2显示，5*7点阵，8位数据
    delay_nms(10);
    LCD_Writecmd(0x38);  //16*2显示，5*7点阵，8位数据
    delay_nms(10);
    LCD_Writecmd(0x08);  //先关显示，后开显示
    delay_nms(10);
    LCD_Writecmd(0x01);  //清除LCD的显示内容
    delay_nms(10);
    LCD_Writecmd(0x06);  //自动右移光标,0x04为左移光标
    delay_nms(10);
    LCD_Writecmd(0x0c);  //显示开，关光标;0x08为关显示
    delay_nms(10);
}
//延时n us
void delay_nus(unsigned long n)
{
    unsigned long j;
    while (n--)
    {
        j = 8;
        while (j--);
    }
}
//延时n ms
void delay_nms(unsigned long n)
{
    while (n--)
    {
        delay_nus(1100);
    }
}
/******************************************************************/
/*                      GPIO配置函数                            */
/******************************************************************/

void A1602_GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE); //开启端口D的时钟
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    //PD0~PD7 推挽输出，不能有漏极输出,除非外有有上拉电阻
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE); //开启端口E的时钟
    GPIO_InitStructure.GPIO_Pin = (GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2);
    //PE.0~2 推挽输出，不能有漏极输出,除非外有有上拉电阻
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}





