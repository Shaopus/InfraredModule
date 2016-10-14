/*
*********************************************************************************************************
*
* ģ������ : ADC����ģ��
* �ļ����� : adc.c
* ��    �� : V1.0
* ˵    �� : ADC������ģ��,��Ҫ��ȡ�����ѹ
*
* �޸ļ�¼ :
*   �汾��  ����        ����     ˵��
*   V1.0    2016-07-28  ShaoPu   ��ʽ����
*
*********************************************************************************************************
*/
#include "stm8l15x.h"

/*
*********************************************************************************************************
* �� �� ��: ADC_Config
* ����˵��: ADC������
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void ADC_Config(void)
{
   
    GPIO_Init(GPIOB,  GPIO_Pin_0,GPIO_Mode_In_FL_No_IT);
    
    /* Initialise and configure ADC1 */
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, ENABLE); 
    ADC_DeInit(ADC1);//��ʼ��ADC1
    ADC_Init(ADC1, ADC_ConversionMode_Single, ADC_Resolution_12Bit, ADC_Prescaler_2);
    ADC_SamplingTimeConfig(ADC1, ADC_Group_SlowChannels, ADC_SamplingTime_384Cycles);

    /* Enable ADC1 */
    ADC_Cmd(ADC1, ENABLE);
    
    /* Enable ADC1 Channel 18 */
    ADC_ChannelCmd(ADC1, ADC_Channel_18, ENABLE);

    /* Enable End of conversion ADC1 Interrupt */
//    ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);

    /* Start ADC1 Conversion using Software trigger*/
//    ADC_SoftwareStartConv(ADC1);
}

/*
*********************************************************************************************************
* �� �� ��: ADC1_GET
* ����˵��: ADC�Ļ�ȡ
* ��    ��: ��
* �� �� ֵ: ad_value:��ȡ��ADֵ
*********************************************************************************************************
*/
static uint16_t ADC1_GET(void)
{
//    FlagStatus flag_status;        //�趨��־λ  SET/RESET
//    uint16_t	ad_value=0;
//    ADC_SoftwareStartConv(ADC1);
//    do
//    {
//        flag_status = ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC);
//    }
//    while(!flag_status);
//    ad_value=ADC_GetConversionValue(ADC1);
//    return ad_value;    
    ADC_SoftwareStartConv(ADC1);//��ʼת��
    while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC)==0);//�Ƿ�ת������
    return ADC_GetConversionValue(ADC1);;//��adcת��ֵ
   
                                        
}

/*
*********************************************************************************************************
* �� �� ��: ADC_Close
* ����˵��: ADC�Ĺر�,�ر�ʱ�Ӻ�����GPIOΪ�͹���ģʽ
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void ADC_Close(void)
{   
    ADC_ClearFlag(ADC1,ADC_FLAG_EOC);
    ADC_ChannelCmd(ADC1, ADC_Channel_18, DISABLE);
    ADC_Cmd(ADC1,DISABLE);
    CLK_PeripheralClockConfig(CLK_Peripheral_ADC1, DISABLE);
    GPIO_Init(GPIOB,  GPIO_Pin_0, GPIO_Mode_Out_OD_Low_Slow);
}

/*
*********************************************************************************************************
* �� �� ��: GET_Batvot
* ����˵��: ��ص����ļ��
* ��    ��: ��
* �� �� ֵ: flag��1 �͵���
                  0 ����
*********************************************************************************************************
*/
uint16_t vot = 0;

uint8_t GET_Batvot(void)
{
    uint8_t flag = 0;
    
    ADC_Config();
    vot = ADC1_GET();
    vot = (vot>>4) & 0xff;
    if(vot < 0x80) //9.5->0xca 9.3->0xc6 9->0xc0 8.8->0xbb 8.5v->0xb5 8v->0xab 7.8->0xa6 7.5->0xa0 7.3->0x9b 7v->0x95 6v->0x80
        flag = 1;
    else
        flag = 0;
    ADC_Close();
    return flag;
}


/***********************************************(END OF FILE) *******************************************/
