
#include "rc522_spi_driver.h"
#include "rc522_handle.h"
#include "rc522.h"
#include "usart1.h"
#include "user_task.h"
/*=================================================================
*               Local Variables
==================================================================*/
uint8_t     access_flag = 0;
uint8_t     k;
uint8_t     i;
uint8_t     j;
uint8_t     b;
uint8_t     q;
uint8_t     en;
uint8_t     ok;
uint8_t     comand;
uint8_t     text1[63] = "STM32F103 Mifare RC522 RFID Card reader 13.56 MHz for KEIL HAL\r";
uint8_t     text2[9] = "Card ID: ";
uint8_t     end[1] = "\r";
uint8_t     txBuffer[18] = "Card ID: 00000000\r";
uint8_t     retstr[10];
uint8_t     rxBuffer[8];
uint8_t     lastID[4];
uint8_t     memID[8] = "82CF8B1A";
uint8_t     str[MFRC522_MAX_LEN];

/*=================================================================
*               Local Functions
==================================================================*/
extern void char_to_hex(uint8_t data) ;

void MFRC522_Module_Init(void)
{
    /*spi driver init*/
    SPI_GPIO_Init();
    SPI_MODUL_Init();

    MFRC522_Init();
}
void MFRC522_Handle(void)
{
    if (!MFRC522_Request(PICC_REQIDL, str))
    {
        if (!MFRC522_Anticoll(str))
        {
            j = 0;
            q = 0;
            b = 9;
            en = 1;
            //printf("\n[RC522 Test] 001\n");
            for (i = 0; i < 4; i++)
            {
                if (lastID[i] != str[i])
                {
                    j = 1;
                }                         // Repeat test
            }

            if (/*j && */en)
            {
                //printf("\n[RC522 Test] 002\n");
                q = 0;
                en = 0;
                for (i = 0; i < 4; i++)
                {
                    lastID[i] = str[i];
                }
                //HAL_UART_Transmit(&huart1, text2, 9, 100);
                for (i = 0; i < 4; i++)
                {
                    char_to_hex(str[i]);
                    txBuffer[b] = retstr[0];
                    b++;
                    txBuffer[b] = retstr[1];
                    b++;
                }
                printf("\n%s", txBuffer);

                ok = 1;
                for (i = 0; i < 8; i++)
                {
                    if (txBuffer[9 + i] != memID[i])
                    {
                        ok = 0;
                    }
                }
            }
        }
        if (ok == 1)
        {
            access_flag = ok;
            ok = 0;
            printf("\n[RC522] brush card success\n");
            app_send_msg(APP_MSG_RC522, sizeof(access_flag), &access_flag);
        }
        else
        {
            access_flag = ok;
            printf("\n[RC522] brush card failed\n");
            app_send_msg(APP_MSG_RC522, sizeof(access_flag), &access_flag);
        }
    }
}

uint8_t hex_to_char(uint8_t data)
{
    uint8_t number;

    if (rxBuffer[data] < 58) { number = (rxBuffer[data] - 48) * 16; }
    else { number = (rxBuffer[data] - 55) * 16; }
    data++;
    if (rxBuffer[data] < 58) { number = number + (rxBuffer[data] - 48); }
    else { number = number + (rxBuffer[data] - 55); }
    return number;
}

// char number to string hex (FF) (Only big letters!)
void char_to_hex(uint8_t data)
{
    uint8_t digits[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};

    if (data < 16)
    {
        retstr[0] = '0';
        retstr[1] = digits[data];
    }
    else
    {
        retstr[0] = digits[(data & 0xF0) >> 4];
        retstr[1] = digits[(data & 0x0F)];
    }
}

// Number to string (16 bit)
void StrTo()
{
    uint32_t    number;
    uint32_t    ret;

    number = 0;
    ret = 0;
    ret = retstr[0] - 0x30;
    number = ret * 1000000;
    ret = retstr[1] - 0x30;
    number = number + ret * 100000;
    ret = retstr[2] - 0x30;
    number = number + ret * 10000;
    ret = retstr[3] - 0x30;
    number = number + ret * 1000;
    ret = retstr[4] - 0x30;
    number = number + ret * 100;
    ret = retstr[5] - 0x30;
    number = number + ret * 10;
    ret = retstr[6] - 0x30;
    number = number + ret;
}

// String to number (32 bit)
void ToStr(uint32_t number)
{
    uint32_t i;

    i = number / 1000000000;
    number = number - i * 1000000000;
    retstr[0] = i + 0x30;

    i = number / 100000000;
    number = number - i * 100000000;
    retstr[1] = i + 0x30;

    i = number / 10000000;
    number = number - i * 10000000;
    retstr[2] = i + 0x30;

    i = number / 1000000;
    number = number - i * 1000000;
    retstr[3] = i + 0x30;

    i = number / 100000;
    number = number - i * 100000;
    retstr[4] = i + 0x30;

    i = number / 10000;
    number = number - i * 10000;
    retstr[5] = i + 0x30;

    i = number / 1000;
    number = number - i * 1000;
    retstr[6] = i + 0x30;

    i = number / 100;
    number = number - i * 100;
    retstr[7] = i + 0x30;

    i = number / 10;
    number = number - i * 10;
    retstr[8] = i + 0x30;

    retstr[9] = number + 0x30;
}

