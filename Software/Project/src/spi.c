/*
*********************************************************************************************************
*
* ģ������ : SPI����ģ��
* �ļ����� : SPI.c
* ��    �� : V1.0
* ˵    �� : SPI������ģ��,��Ҫ����CC1101
*
* �޸ļ�¼ :
*   �汾��  ����        ����     ˵��
*   V1.0    2016-07-27  ShaoPu   ��ʽ����
*
*********************************************************************************************************
*/
#include "spi.h"
#include "stm8l15x.h"

/*
*********************************************************************************************************
* �� �� ��: SPI_Initial
* ����˵��: SPI�ĳ�ʼ��
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void SPI_Initial(void)
{
    /*ʹ��SPI1��ʱ��*/
    CLK_PeripheralClockConfig(CLK_Peripheral_SPI1, ENABLE);
    
    /*SPI1��λ*/
    SPI_DeInit(SPI1);
    
    /*
    SPI�ĳ�ʼ����ָ�����ݴ����MSBλ����LSBλ��ʼ�����ݴ����MSBλ��ʼ;���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ2��
                 ����SPI����ģʽ:����Ϊ��SPI;ѡ���˴���ʱ�ӵ���̬��ʱ�����յͣ���һ����Եץ���ݣ�
                 ����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��;NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ�������������CS 
    */  
    SPI_Init(SPI1,SPI_FirstBit_MSB, SPI_BaudRatePrescaler_2,
             SPI_Mode_Master, SPI_CPOL_Low, SPI_CPHA_1Edge,
             SPI_Direction_2Lines_FullDuplex, SPI_NSS_Soft ,0x07);	//crcУ��Ĭ���ǹ��ŵ�
                                                                 
    /*ʹ��SPI1*/	
    SPI_Cmd(SPI1,ENABLE);
    
    /*����SPI1�ӿڵ�GPIO*/
    GPIO_Init(GPIOB, GPIO_Pin_7,GPIO_Mode_In_PU_No_IT);//MISO
    GPIO_Init(GPIOB, GPIO_Pin_5 | GPIO_Pin_6,GPIO_Mode_Out_PP_High_Slow);//SCK,MOSI
    GPIO_Init(GPIOB, GPIO_Pin_4,GPIO_Mode_Out_PP_High_Slow);//CSN
    GPIO_Init(GPIOB, GPIO_Pin_3,GPIO_Mode_In_PU_No_IT);	//GDO0
    GPIO_Init(GPIOD, GPIO_Pin_4,GPIO_Mode_In_PU_No_IT);	//GDO2
}

/*
*********************************************************************************************************
* �� �� ��: SPI_ExchangeByte
* ����˵��: SPI�Ķ�д
* ��    ��: input����Ҫ���͵�SPI����
* �� �� ֵ: ��ȡ��SPI����
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
