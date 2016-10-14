/*
*********************************************************************************************************
*
* ģ������ : ������
* �ļ����� : main.c
* ��    �� : V1.0
* ˵    �� : 
*            ��Ҫʵ�ֵ͹���(Active-Halt)ģʽ�£�ʹ��RTC+�жϻ��ѵ�Ƭ��������CC1101�������
*            ��Active-Haltģʽ�£���ʱ�Ӻ�CPU���Լ��󲿷ֵ����趼ֹͣ�ˣ�ֻʣ��LSE��LSIΪһ���ֵ������ṩʱ��
*            ����Active-Haltģʽ���е����������SWIM��beeper��IWDG��RTC��LCD��
*            ����Active-Haltģʽ����ͨ��halt��䡣
*            �˳���ͨ���ⲿ�жϡ�RTC�͸�λ��
*
*            ʵ�����¹��ܣ���Ϊ�˱�֤ͨ�ţ���Ϣ�����Σ����300�������ң�
*                          1����һ���ϵ�ʱ���������ÿ�α���������ʱ���������������豨�����͡�
*                          2������Ĭ��--����--���������ź�Ϊ�ͣ���������澯�������ͱ�����Ϣ������(���ⲿ�жϻ��ѣ������ź�Ϊ��)��
*                          3�����⴫��������ÿ��180���ӷ��͹���״̬�����ء�
*                          4����������״̬�£�һ�������ε�������ÿ4�������������������������㣬�������ͣ�
*
*            �ڸ�ģʽ�£����Դ�������:1.3uA(��Ƭ��+CC1101˯��ģʽ+KEY�Ͽ�����LDO)��
*                        ���Դ�������:2.3uA(��Ƭ��+CC1101˯��ģʽ+KEY�պϣ���LDO)����������Ϊ3.3M  3/3.3M = 0.9uA
*                        �жϻ�RTC����ʱ���͵���:ԼΪ13mA���ϡ�
*                        ���϶���VICTOR VC890C+���ñ������
* �޸ļ�¼ :
*   �汾��  ����        ����     ˵��
*   V1.0    2016-07-29  ShaoPu   ��ʽ����
*
*********************************************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm8l15x.h"
#include "delay.h"
#include "CC1101.h"
#include "spi.h"
#include "adc.h"

/* Private typedef -----------------------------------------------------------*/

/*�������ݰ��ṹ��*/
typedef struct 
{
    uint8_t len;//���ݰ�����
    uint8_t ctrl;//���ݰ�������
    uint8_t type;//�豸����
    uint8_t number[5];//�豸���к�
    uint8_t alarm;//������Ϣ
}TXBUF;

/*ģ�鹤��״̬*/
typedef enum
{
    SYS_RTC      = 0,
    SYS_EXTI     = 1
}SYS;

/*CC1101�������ݰ�����*/
TXBUF txbuf;

/* Private define ------------------------------------------------------------*/

/*�豸���ͺ��豸ID�����Flash��ʼ��ַ*/
#define start_addr      0xbff0  

/*20��������RTCһ��*/
#define HEAT_BTIME	44531	//20min	        //35000 //15min

/*2��������RTCһ��*/
//#define HEAT_BTIME	223	//2min

/*180���ӷ���������*/
#define HEAT_CNT	8	//20min*9 = 180min---3hour

/*720���ӷ��͵�����*/
#define BATTERY_CNT	4	//180min*4 = 720min---12hour

/*�жϷ���ʱ���������ļ��ʱ��*/
#define KEY_RECONTIME	5	//135ms

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/*ģ�鹤��״̬����ʼ��Ϊ����״̬*/
uint8_t work_sta = SYS_RTC;

/*�жϻ���״̬�µģ���ʱ���ͼ���*/
uint8_t working_cnt = 0;

/*RTC����״̬�µģ���ʱ���ͼ���*/
uint8_t rtc_cnt = 0;

/*���Ѻ󣬵�ǰKEY�ĵ�ƽ���ڻ��Ѻ���*/
uint8_t key = 0;

/*���Ѻ���һ��KEY�ĵ�ƽ�����豸����ʱ���*/
volatile uint8_t pre_key = 0;

/*�жϻ��ѱ�־λ*/
volatile uint8_t ext_flag = 0;

/*RTC����ʱ��*/
uint16_t tm = 0;

/*����������ֵ*/
uint8_t heat_cnt = 0;

/*����������ֵ*/
uint8_t battery_cnt = 0;

/*�͵�����־λ*/
uint8_t ucBatvot = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*
*********************************************************************************************************
* �� �� ��: TXBUF_Init
* ����˵��: ���ݰ���ʼ��
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void TXBUF_Init(void)
{
    uint8_t i;
    uint8_t * p;
    
    /*���ݰ�����:8���ֽ�*/
    txbuf.len = 0x08;
    
    /*���ݰ�������*/
    txbuf.ctrl = 0x00;

    /*��Flash�ж�ȡ�豸���ͺ��豸ID*/
    p = (uint8_t *)&(txbuf.type);
    for(i = 0;i<6;i++)
            *(p+i) = FLASH_ReadByte(start_addr+i);
    
    /*�豸������Ϣ*/
    txbuf.alarm = 0x00;
}

/*
*********************************************************************************************************
* �� �� ��: CLK_Config
* ����˵��: ����ʱ��
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void CLK_Config(void)  
{
    /*ʹ��ʱ���л�*/
    CLK_SYSCLKSourceSwitchCmd(ENABLE);
    
    /*ѡ���ڲ�����ʱ����Ϊʱ��Դ*/
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
    
    /* ����ϵͳʱ�ӷ�Ƶ: 16*/
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_16);
    
    /* �ȴ�ʱ���ȶ�*/
    while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
    {     
    }
}

/*
*********************************************************************************************************
* �� �� ��: RTC_Config
* ����˵��: ����RTC
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void RTC_Config(void)
{
    CLK_LSICmd(ENABLE);
    
    /*�ȴ��ڲ�ʱ������׼��*/
    while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
    
    /*����RTC����ģ���ʱ��*/
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
    
    /*����RTCʱ�ӣ�ʹ���ڲ�����ʱ��38K��64��Ƶ 38000/64 = 593.75 */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_64);
    
    /*the wake up Unit must be disabled (if enabled) using RTC_WakeUpCmd(Disable).*/
    RTC_WakeUpCmd(DISABLE);
    
    /*Configures the RTC wakeup timer_step:38k/64/16=37.109375KHz t=27ms */
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
       
    /*����RTC�ж����ȼ�*/
    ITC_SetSoftwarePriority(RTC_IRQn, ITC_PriorityLevel_3);
          
    /*ʹ��RTC�ж�*/
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
         
}

/*
*********************************************************************************************************
* �� �� ��: mcu_sleep
* ����˵��: ����RTC����ʱ��
* ��    ��: cnt:ʱ��ֵ(0-65535)       cnt*27ms
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void mcu_sleep(uint16_t cnt)
{
    /*����RTC������*/
    RTC_SetWakeUpCounter(cnt);
    
    /*ʹ��RTC����*/
    RTC_WakeUpCmd(ENABLE);	
}

/*
*********************************************************************************************************
* �� �� ��: GPIO_LowPower_Config
* ����˵��: ���õ͹���ģʽ�µ�GPIO---STM8L151G4
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void GPIO_LowPower_Config(void) 
{
  
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);
    GPIO_DeInit(GPIOD); 
    
    /*�����õ�IOȫ�����ó� GPIO_Mode_Out_OD_Low_Slow*/
    GPIO_Init(GPIOA,  GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5, GPIO_Mode_Out_OD_Low_Slow);
    GPIO_Init(GPIOB,  GPIO_Pin_All, GPIO_Mode_Out_OD_Low_Slow);
    GPIO_Init(GPIOC,  GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6, GPIO_Mode_Out_OD_Low_Slow);	
    GPIO_Init(GPIOD,  GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4, GPIO_Mode_Out_OD_Low_Slow);    

    /*ʹ����UltraLowPowerģʽ�Ŀ��ٻ���*/
    PWR_FastWakeUpCmd(ENABLE);
    
    /*ʹ��UltraLowPowerģʽ*/
    PWR_UltraLowPowerCmd(ENABLE); 
}

/*
*********************************************************************************************************
* �� �� ��: EXTI_Init
* ����˵��: ��ʼ���ⲿ�ж�---GPIOB2
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void EXTI_Init(void)
{
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_In_FL_IT);
    EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Rising_Falling);
}

/*
*********************************************************************************************************
* �� �� ��: EXTI_UnInit
* ����˵��: �����ⲿ�жϣ�GPIO��Ϊ�͹���ģʽ---GPIOB2
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void EXTI_UnInit(void)
{
    EXTI_DeInit();
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_Out_OD_Low_Slow);
}

/*
*********************************************************************************************************
* �� �� ��: ext_handle
* ����˵��: �жϵ��ú���
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void ext_handle(void)
{
    /*�жϱ�־λ�� 1*/
    ext_flag = 1;

    EXTI_UnInit();
}

/*
*********************************************************************************************************
* �� �� ��: CC1101_Init
* ����˵��: CC1101��ʼ��
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101_Init(void)
{
    /*SPI��ʼ��*/
    SPI_Initial();
    
    /*CC1101��ʼ��*/
    CC1101Init();
    
    /*CC1101�����������*/
    CC1101ClrTXBuff();
    
    /*����CC1101Ϊ����ģʽ*/
    CC1101SetTRMode(TX_MODE);
    
    /*CC1101����͹���ģʽ*/
    CC1101SetIdle();		
    CC1101Sleep();
    CC_CSN_HIGH();
}

/*
*********************************************************************************************************
* �� �� ��: op
* ����˵��: CC1101�������ݰ�����
* ��    ��: key��KEY��ƽ��Ϣ
*           flag��������Ϣ
* �� �� ֵ: ��
*********************************************************************************************************
*/
void op(uint8_t key,uint8_t flag)
{
    /*���ݰ�������Ϣ����*/
    txbuf.alarm = 0x00;
    
    /*��ʼ��SPI��CC1101 ����������ݰ�������CC1101Ϊ����ģʽ*/
    SPI_Initial();
    CC1101Init();
    CC1101ClrTXBuff();
    CC1101SetTRMode(TX_MODE);			
 
    /*�ж�KEY��ƽ��Ϣ�����Ϊ1�����ͱ�����Ϣ
                       ���Ϊ0������������Ϣ
    */
    if(key!=0)
        txbuf.alarm = 0xf0;
    else
        txbuf.alarm = 0x00;

    /*�жϵ�����Ϣ������͵�������FlagΪ1�����͵͵�����Ϣ*/
    if(flag != 0)
        txbuf.alarm |= 0x08;
 
    /*CC1101�������ݰ�������Ϊ9*/ 
    CC1101SendPacket((INT8U *)&txbuf,9, ADDRESS_CHECK);
 
    /*������ʱ*/ 
    DelayT_ms(10);
    
    /*������ɺ�CC1101����͹���ģʽ*/
    CC1101SetIdle();		
    CC1101Sleep();
    CC_CSN_HIGH();
}

/*
*********************************************************************************************************
* �� �� ��: run
* ����˵��: RTC���жϻ��Ѵ�����
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void run(void)
{
    /*�ر�ȫ���ж�*/
    disableInterrupts();
    
    /*RTC���ѹر�*/
    RTC_WakeUpCmd(DISABLE);
    
    /*���Ѻ󣬽�KEY���ó����жϵ��������룬��ȡ��ƽ*/
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_In_PU_No_IT);
    
    /*��ȡ���Ѻ�ĵ�ǰKEY��ƽ*/
    key = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2);
    
    /*�ж��Ƿ����ⲿ�жϻ���
	  ����ǣ�״̬��ֱ�ӽ����жϻ��ѵĹ���״̬
    */
    if(ext_flag)
    {
        heat_cnt = 0;//����������������
        ext_flag = 0;//�жϱ�־λ����
        work_sta = SYS_EXTI;//����״̬Ϊ�ⲿ����״̬
        working_cnt = 0;//��ʱ���ͼ���������
    }
    
    /*�ж�KEY��ֵ�Ƿ�����һ����ͬ
      �������ͬ��˵�����жϻ��ѣ�״̬�����빤��״̬�����浱ǰkeyֵ��Ȼ��CC1101�������ݰ�һ��
      ��ͬ�������˵��RTC���ѵ�����£����账��
    */
    if(key!=pre_key)
    {
        heat_cnt = 0;//����������������
        working_cnt = 0;//�жϱ�־λ����	
        work_sta = SYS_EXTI;//����״̬Ϊ�ⲿ����״̬
        pre_key = key;//���浱ǰ��ƽ
        op(key,0);//��һ�η������ݰ�
    }
    
    /*�жϻ��Ѻ�Ĺ���״̬*/
    switch (work_sta)
    {
        /*RTC����*/
        case SYS_RTC:
            /*����RTC����ʱ��*/
            tm = HEAT_BTIME; //����20���� 
            
            /*�������������ж��Ƿ񵽴�180����*/
            if(heat_cnt < HEAT_CNT)
            {
                heat_cnt ++;
            }
            else
            { 
                /*����RTC���Ѻ󣬵ȴ�275ms���� ���͵�1�����ݰ�*/
                if(rtc_cnt == 0)
                {                    
                    rtc_cnt = 1;
                    
                    ucBatvot = 0;
                    /*�͵����������������ж��Ƿ��ǵ���720����
                      ����ǣ����ʹ��е�����Ϣ�����ݰ�
                      ������ǣ������������ݰ�
                    */
                    battery_cnt++;
                    if(battery_cnt>=BATTERY_CNT)
                    {
                        battery_cnt = 0;
                        ucBatvot = GET_Batvot();
                    }
                    
                    op(key,ucBatvot);//��1�η������ݰ�                   
                    //����270ms
                    tm = KEY_RECONTIME;
                }
                else
                {                    
                    rtc_cnt = 0;//��ʱ���ͼ���������
                    
                    /*180minһ����������������������ݰ�*/
                    heat_cnt = 0;
                                       
                    op(key,ucBatvot);                
                }
            }
        break;
        
        /*�жϻ���*/
        case SYS_EXTI:
          
            /*�����жϻ��Ѻ󣬵ȴ�275ms���� ���͵ڶ������ݰ�*/
            if(working_cnt == 0)
            {
                working_cnt = 1;
                //����135ms
                tm = KEY_RECONTIME;
            }
            else
            {
                working_cnt = 0;//��ʱ���ͼ���������
                work_sta = SYS_RTC;//����״̬�л�ΪRTC����״̬
                
                op(key,0);//�ڶ��η������ݰ�
                
               /*����RTC����ʱ��*/
                tm = HEAT_BTIME; //����20����
                //���߷��ͣ�������IDLEģʽ
            }		
         break;
    }
    
    /*��������KEYΪ�ж�ģʽ*/
    EXTI_Init();
    
    /*��������RTC����ʱ�䣬ʹ��ȫ���жϣ�����active haltģʽ*/
    mcu_sleep(tm);
    enableInterrupts();		
    halt();
}

void main(void)
{  
  
    DelayT_ms(500); 
    
    disableInterrupts();
    
    /*���ݰ���ʼ��*/
    TXBUF_Init();
    /*����ʱ����Ƶ*/ 
//    CLK_Config(); //�򲻴���˯������ʱ����ֵӰ�첻�󣬶���0.735mA
    /*��ȡ��ص���״̬*/    
    ucBatvot = GET_Batvot();
    
    /*�ϵ緢�ʹ����������ݰ�����������*/  
    op(key,ucBatvot);
    DelayT_ms(50);     
    op(key,ucBatvot); 
    DelayT_ms(50);
    op(key,ucBatvot);
    
    /*��KEY���ó����жϵ��������룬��ȡ��ƽ������ƽ���浽pre_key��*/
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_In_PU_No_IT);
    pre_key = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2);
    
    /*����ϵͳʱ����Halt��Active-Haltģʽ��*/
    CLK_HaltConfig(CLK_Halt_FastWakeup,ENABLE); 
    
    /*�͹���ģʽ��GPIO��ʼ��*/
    GPIO_LowPower_Config();
    
    /*CC1101��ʼ����RTC��ʼ�����жϳ�ʼ��������RTC����ʱ�䣬ʹ��ȫ���жϣ�����activ haltģʽ*/
    CC1101_Init();     
    RTC_Config();
    EXTI_Init();
    mcu_sleep(HEAT_BTIME);
    enableInterrupts();
    
    halt();
    
    while (1)
    {	
        run(); 
    }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/***********************************************(END OF FILE) *******************************************/
