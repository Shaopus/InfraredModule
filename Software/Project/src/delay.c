/*
*********************************************************************************************************
*
* ģ������ : ��ʱ����ģ��
* �ļ����� : delay.c
* ��    �� : V1.0
* ˵    �� : ��ʱ������ģ��
*
* �޸ļ�¼ :
*   �汾��  ����        ����     ˵��
*   V1.0    2016-07-28  ShaoPu   ��ʽ����
*
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/

#include "stm8l15x_clk.h"

/*
*********************************************************************************************************
* �� �� ��: DelayT_ms
* ����˵��: ����ȷ�ĺ��뼶��ʱ����������16MHzʱ��
* ��    ��: nCount������
* �� �� ֵ: ��
*********************************************************************************************************
*/ 
void DelayT_ms(unsigned int nCount)  
{  
    uint16_t i=0,j=0;;  
    for(i=0;i<nCount;i++)  
    {  
        for(j=0;j<430;j++)  //2mhz--430
        {;}  
    }  
} 

/*
*********************************************************************************************************
* �� �� ��: delay_ms
* ����˵��: ��ȷ�ĺ��뼶��ʱ����������TIM4����
* ��    ��: n_ms:��ʱʱ��(ms)
* �� �� ֵ: ��
*********************************************************************************************************
*/ 
void delay_ms(u16 n_ms)
{
/* Init TIMER 4 */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

/* Init TIMER 4 prescaler: / (2^3) = /8 */
  TIM4->PSCR = 3;

/* HSI div by 1 --> Auto-Reload value: 16M/8/8 = 250000, 250000/1k = 250*/
  TIM4->ARR = 250;
  
/* Counter value: 2, to compensate the initialization of TIMER*/
  TIM4->CNTR = 2;

/* clear update flag */
  TIM4->SR1 &= ~TIM4_SR1_UIF;

/* Enable Counter */
  TIM4->CR1 |= TIM4_CR1_CEN;

  while(n_ms--)
  {
    while((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;//�ж�ʱ���׳�����
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

/* Disable Counter */
  TIM4->CR1 &= ~TIM4_CR1_CEN;
}

/*
*********************************************************************************************************
* �� �� ��: delay_10us
* ����˵��: ��ȷ��10΢�뼶��ʱ����������TIM4����
* ��    ��: n_10us:��ʱʱ��(10us)
* �� �� ֵ: ��
*********************************************************************************************************
*/ 
void delay_10us(u16 n_10us)
{
/* Init TIMER 4 */
  CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, ENABLE);

/* prescaler: / (2^0) = /1 */
  TIM4->PSCR = 0;

/* SYS_CLK_HSI_DIV1 Auto-Reload value: 16M /8/ 1 = 2M, 2M / 100k = 160 */
  TIM4->ARR = 20;

/* Counter value: 10, to compensate the initialization of TIMER */
  TIM4->CNTR = 10;

/* clear update flag */
  TIM4->SR1 &= ~TIM4_SR1_UIF;

/* Enable Counter */
  TIM4->CR1 |= TIM4_CR1_CEN;

  while(n_10us--)
  {
    while((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;//�ж�ʱ���׳�����
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

/* Disable Counter */
  TIM4->CR1 &= ~TIM4_CR1_CEN;
 CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);
}



/***********************************************(END OF FILE) *******************************************/
