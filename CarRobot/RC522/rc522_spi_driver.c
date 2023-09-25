#include "rc522.h"
#include "stm32f10x_spi.h"
#include "stm32f10x_gpio.h"



/**
* @brief SPI_FLASH ???
* @param ?
* @retval ?
*/
void SPI_GPIO_Init(void)
{
//SPI_InitTypeDef SPI_InitStructure;
    GPIO_InitTypeDef GPIO_InitStructure;

    /* ?? SPI ?? */
    FLASH_SPI_APBxClock_FUN(FLASH_SPI_CLK, ENABLE);

    /* ?? SPI ??????? */
    FLASH_SPI_CS_APBxClock_FUN(FLASH_SPI_CS_CLK | FLASH_SPI_SCK_CLK |
                               FLASH_SPI_MISO_PIN | FLASH_SPI_MOSI_PIN, ENABLE);

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



