#include "delay.h"
#include "sys.h"
#include "usart.h"
#include "Si7021.h"
#include "myiic.h"
#include "1602.h"


int main(void)
{
    u8 i = 0;
    char temp_buf[11] = {0};
    char humi_buf[11] = {0};
    unsigned short temp_value = 0;
    unsigned short humi_value = 0;

    delay_init();           //延时函数初始化
    uart_init(115200);      //串口1初始化波特率为115200
    IIC_Init();             //IIC初始化
    A1602_GPIO_Configuration();
    LCD_Init();
    //LCD_write_str(0, 0, "hello a1602");

    while (1)
    {
        //LCD_write_str(0, 0, "hello a1602");

        //每50ms读取一次数据
        measure_Si7021();

        i++;
        delay_ms(800);
        if (i == 20)
        {
            //串口每1s打印一次数据
            printf("\r\nTemp:%d\r\n", temp_value); //打印温度数据，保留两位小数
            printf("Humi:%d\r\n", humi_value); //打印湿度数据，保留两位小数
            i = 0;
        }
        LCD_write_str(0, 0, "Temp: ");
        temp_value = (unsigned short)(TEMP_buf * 10) % 1000;
        temp_buf[0] = temp_value / 100 + '0';
        temp_buf[1] = temp_value / 10 % 10 + '0';
        temp_buf[2] = '.';
        temp_buf[3] = temp_value % 10 + '0';
        temp_buf[4] = 0xdf;
        temp_buf[5] = 'C';
        temp_buf[6] = ' ';
        temp_buf[7] = ' ';
        temp_buf[8] = ' ';
        temp_buf[9] = ' ';
        temp_buf[10] = 0;

        LCD_write_str(6, 0, temp_buf);

        LCD_write_str(0, 1, "Humi: ");
        humi_value = (unsigned short)(Humi_buf * 10) % 1000;
        humi_buf[0] = humi_value / 100 + '0';
        humi_buf[1] = humi_value / 10 % 10 + '0';
        humi_buf[2] = '.';
        humi_buf[3] = humi_value % 10 + '0';
        humi_buf[4] = 'R';
        humi_buf[5] = 'H';
        humi_buf[6] = '%';
        humi_buf[7] = ' ';
        humi_buf[8] = ' ';
        humi_buf[9] = ' ';
        humi_buf[10] = 0;

        LCD_write_str(6, 1, humi_buf);
    }
}
