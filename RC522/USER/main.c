/******************** (C) COPYRIGHT 2011 野火嵌入式开发工作室 ********************
 * 文件名  ：main.c
 * 描述    ：将c库中的printf()函数实现到串口1(USART1)。这样我们就可以用printf()将
 *           调试信息通过串口打印到电脑上。         
 * 实验平台：野火STM32开发板
 * 库版本  ：ST3.0.0
 *
 * 作者    ：fire  QQ: 313303034 
 * 博客    ：firestm32.blog.chinaunix.net
**********************************************************************************/

#include "stm32f10x.h"
#include "usart1.h"

#include "rc522.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"

uint8_t 	k;
uint8_t 	i;
uint8_t 	j;
uint8_t 	b;
uint8_t 	q;
uint8_t 	en;
uint8_t 	ok;
uint8_t 	comand;
uint8_t		text1[63] = "STM32F103 Mifare RC522 RFID Card reader 13.56 MHz for KEIL HAL\r";
uint8_t		text2[9] = "Card ID: ";
uint8_t		end[1] = "\r";
uint8_t		txBuffer[18] = "Card ID: 00000000\r";
uint8_t 	retstr[10];
uint8_t 	rxBuffer[8];
uint8_t		lastID[4];
uint8_t		memID[8] = "9C55A1B5";
uint8_t		str[MFRC522_MAX_LEN];		

/**
* @brief SPI_FLASH ???
* @param ?
* @retval ?
*/
void SPI_GPIO_Init(void)
{
SPI_InitTypeDef SPI_InitStructure;
GPIO_InitTypeDef GPIO_InitStructure;
 
 /* ?? SPI ?? */
 FLASH_SPI_APBxClock_FUN ( FLASH_SPI_CLK, ENABLE );
 
 /* ?? SPI ??????? */
 FLASH_SPI_CS_APBxClock_FUN ( FLASH_SPI_CS_CLK|FLASH_SPI_SCK_CLK|
 FLASH_SPI_MISO_PIN|FLASH_SPI_MOSI_PIN, ENABLE );
 
 /* ?? SPI ? CS ??,?? IO ?? */
 GPIO_InitStructure.GPIO_Pin = FLASH_SPI_CS_PIN;
 GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 GPIO_Init(FLASH_SPI_CS_PORT, &GPIO_InitStructure);
 
 /* ?? SPI ? SCK ??*/
 GPIO_InitStructure.GPIO_Pin = FLASH_SPI_SCK_PIN;
 GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
 GPIO_Init(FLASH_SPI_SCK_PORT, &GPIO_InitStructure);
 
 /* ?? SPI ? MF103-????*/
 GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MISO_PIN;
 GPIO_Init(FLASH_SPI_MISO_PORT, &GPIO_InitStructure);
 
 /* ?? SPI ? MOSI ??*/
 GPIO_InitStructure.GPIO_Pin = FLASH_SPI_MOSI_PIN;
 GPIO_Init(FLASH_SPI_MOSI_PORT, &GPIO_InitStructure);
 
 /* ???? FLASH: CS ?????*/
 FLASH_SPI_CS_HIGH();
 //?????,???? SPI ???????
}
void SPI_MODUL_Init(void)
{
 
 SPI_InitTypeDef SPI_InitStructure;
 /* SPI ???? */
 // FLASH ?? ?? SPI ?? 0 ??? 3,???? CPOL CPHA
 SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
 SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
 SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
 SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
 SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
 SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
 SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;
 SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
 SPI_InitStructure.SPI_CRCPolynomial = 7;
 SPI_Init(FLASH_SPIx, &SPI_InitStructure);
 
 /* ?? SPI */
 SPI_Cmd(FLASH_SPIx, ENABLE);
 }


uint8_t hex_to_char(uint8_t data) {
	uint8_t number;
	
	if (rxBuffer[data] < 58) number = (rxBuffer[data]-48)*16; else number = (rxBuffer[data]-55)*16;
	data++;
	if (rxBuffer[data] < 58) number = number+(rxBuffer[data]-48); else number = number+(rxBuffer[data]-55);
	return number;
}

// char number to string hex (FF) (Only big letters!)
void char_to_hex(uint8_t data) {
	uint8_t digits[] = {'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
	if (data < 16) {
		retstr[0] = '0';
		retstr[1] = digits[data];
	} else {
		retstr[0] = digits[(data & 0xF0)>>4];
		retstr[1] = digits[(data & 0x0F)];
	}
}

// Number to string (16 bit)
void StrTo() {
	uint32_t	number;
	uint32_t 	ret;
	
	number = 0;
	ret = 0;
	ret = retstr[0]-0x30;
	number = ret*1000000;
	ret = retstr[1]-0x30;
	number = number+ret*100000;
	ret = retstr[2]-0x30;
	number = number+ret*10000;
	ret = retstr[3]-0x30;
	number = number+ret*1000;
	ret = retstr[4]-0x30;
	number = number+ret*100;
	ret = retstr[5]-0x30;
	number = number+ret*10;
	ret = retstr[6]-0x30;
	number = number+ret;
}

// String to number (32 bit)
void ToStr(uint32_t number) {
	uint32_t i;
	
	i = number/1000000000;
	number = number-i*1000000000;
	retstr[0] = i+0x30;
	
	i=number/100000000;
	number=number-i*100000000;
	retstr[1] = i+0x30;
	
	i = number/10000000;
	number = number-i*10000000;
	retstr[2] = i+0x30;
	
	i = number/1000000;
	number = number-i*1000000;
	retstr[3] = i+0x30;
	
	i = number/100000;
	number = number-i*100000;
	retstr[4] = i+0x30;
	
	i = number/10000;
	number = number-i*10000;
	retstr[5] = i+0x30;
	
	i = number/1000;
	number = number-i*1000;
	retstr[6] = i+0x30;
	
	i = number/100;
	number = number-i*100;
	retstr[7] = i+0x30;
	
	i = number/10;
	number = number-i*10;
	retstr[8] = i+0x30;
	
	retstr[9] = number+0x30;
}

/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  : 无
 * 输出  ：无
 */
int main(void)
{  
	/* 配置系统时钟为 72M */      
  SystemInit();

  /* USART1 config 115200 8-N-1 */
	USART1_Config();
	
	

	printf("\r\n this is a printf demo \r\n");
		
	USART1_printf(USART1, "\r\n This is a USART1_printf demo \r\n");

	USART1_printf(USART1, "\r\n ("__DATE__ " - " __TIME__ ") \r\n"); 
	SPI_GPIO_Init();
  SPI_MODUL_Init();
	
	USART1_printf(USART1, "\r\n RC522 init \r\n");
	MFRC522_Init();
	while(1)
	{
  if (!MFRC522_Request(PICC_REQIDL, str)) {
			if (!MFRC522_Anticoll(str)) {
				j = 0;
				q = 0;
				b = 9;
				en = 1;

				for (i=0; i<4; i++) if (lastID[i] != str[i]) j = 1;								// Repeat test
				
				if (j && en) {
					q = 0;
					en = 0;
					for (i=0; i<4; i++) lastID[i] = str[i];
					//HAL_UART_Transmit(&huart1, text2, 9, 100);				
					for (i=0; i<4; i++) {
						char_to_hex(str[i]);
						txBuffer[b] = retstr[0];
						b++;
						txBuffer[b] = retstr[1];
						b++;
					}
					USART1_printf(USART1, "\n%s" ,txBuffer);
					
					ok = 1;
					for (i=0; i<8; i++) 
					    if (txBuffer[9+i] != memID[i]) 
								ok = 0;
//					led(1);
				}
				
				//led(1);
			}
		}	
  }
  while (1)
  {
      
  }
}


/******************* (C) COPYRIGHT 2011 野火嵌入式开发工作室 *****END OF FILE****/
