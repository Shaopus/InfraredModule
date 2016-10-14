/*
*********************************************************************************************************
*
* 模块名称 : SPI驱动模块
* 文件名称 : SPI.c
* 版    本 : V1.0
* 说    明 : SPI的驱动模块,主要驱动CC1101
*
* 修改记录 :
*   版本号  日期        作者     说明
*   V1.0    2016-07-27  ShaoPu   正式发布
*
*********************************************************************************************************
*/
#include "spi.h"
#include "stm8l15x.h"

/*
*********************************************************************************************************
* 函 数 名: SPI_Initial
* 功能说明: SPI的初始化
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
void SPI_Initial(void)
{
    /*使能SPI1的时钟*/
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
    
    /*SPI1复位*/
    SPI_DeInit(SPI1);
    
    /*
    SPI的初始化：指定数据传输从MSB位还是LSB位开始：数据传输从MSB位开始;定义波特率预分频的值:波特率预分频值为2，
                 设置SPI工作模式:设置为主SPI;选择了串行时钟的稳态：时钟悬空低，第一个边缘抓数据，
                 设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工;NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理：软件控制CS 
    */  
    SPI_Init(SPI1,SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2,
             SPI_Mode_Master, SPI_CPOL_Low, SPI_CPHA_1Edge,
             SPI_Direction_2Lines_FullDuplex, SPI_NSS_Soft ,0x07);	//crc校验默认是关着的
                                                                 
    /*使能SPI1*/	
    SPI_Cmd(SPI1,ENABLE);
    
    /*配置SPI1接口的GPIO*/
    GPIO_Init(GPIOB, GPIO_Pin_7,GPIO_Mode_In_PU_No_IT);//MISO
    GPIO_Init(GPIOB, GPIO_Pin_5 | GPIO_Pin_6,GPIO_Mode_Out_PP_High_Slow);//SCK,MOSI
    GPIO_Init(GPIOB, GPIO_Pin_4,GPIO_Mode_Out_PP_High_Slow);//CSN
    GPIO_Init(GPIOB, GPIO_Pin_3,GPIO_Mode_In_PU_No_IT);	//GDO0
    GPIO_Init(GPIOD, GPIO_Pin_4,GPIO_Mode_In_PU_No_IT);	//GDO2
}

/*
*********************************************************************************************************
* 函 数 名: SPI_ExchangeByte
* 功能说明: SPI的读写
* 形    参: input：需要发送的SPI数据
* 返 回 值: 读取到SPI数据
*********************************************************************************************************
*/
INT8U SPI_ExchangeByte(INT8U input)
{
    /*wait for last transmitt finishing*/
    while (SPI_GetFlagStatus(SPI1,SPI_FLAG_TXE) == RESET);
    SPI_SendData( SPI1,input );
    
    /*wait for receiving a byte*/
    while (SPI_GetFlagStatus(SPI1,SPI_FLAG_RXNE) == RESET);
    return SPI_ReceiveData(SPI1 );
}


/***********************************************(END OF FILE) *******************************************/
