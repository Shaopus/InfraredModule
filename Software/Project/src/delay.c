/*
*********************************************************************************************************
*
* 模块名称 : 延时驱动模块
* 文件名称 : delay.c
* 版    本 : V1.0
* 说    明 : 延时的驱动模块
*
* 修改记录 :
*   版本号  日期        作者     说明
*   V1.0    2016-07-28  ShaoPu   正式发布
*
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/

#include "stm8l15x_clk.h"

/*
*********************************************************************************************************
* 函 数 名: DelayT_ms
* 功能说明: 不精确的毫秒级延时函数函数，16MHz时钟
* 形    参: nCount：毫秒
* 返 回 值: 无
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
* 函 数 名: delay_ms
* 功能说明: 精确的毫秒级延时函数函数，TIM4驱动
* 形    参: n_ms:延时时间(ms)
* 返 回 值: 无
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
    while((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;//中断时容易出不来
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

/* Disable Counter */
  TIM4->CR1 &= ~TIM4_CR1_CEN;
}

/*
*********************************************************************************************************
* 函 数 名: delay_10us
* 功能说明: 精确的10微秒级延时函数函数，TIM4驱动
* 形    参: n_10us:延时时间(10us)
* 返 回 值: 无
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
    while((TIM4->SR1 & TIM4_SR1_UIF) == 0) ;//中断时容易出不来
    TIM4->SR1 &= ~TIM4_SR1_UIF;
  }

/* Disable Counter */
  TIM4->CR1 &= ~TIM4_CR1_CEN;
 CLK_PeripheralClockConfig(CLK_Peripheral_TIM4, DISABLE);
}



/***********************************************(END OF FILE) *******************************************/
