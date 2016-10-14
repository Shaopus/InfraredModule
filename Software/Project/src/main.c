/*
*********************************************************************************************************
*
* 模块名称 : 主函数
* 文件名称 : main.c
* 版    本 : V1.0
* 说    明 : 
*            主要实现低功耗(Active-Halt)模式下，使用RTC+中断唤醒单片机，控制CC1101发射命令。
*            在Active-Halt模式下，主时钟和CPU，以及大部分的外设都停止了，只剩下LSE或LSI为一部分的外设提供时钟
*            能在Active-Halt模式运行的外设包括：SWIM、beeper、IWDG、RTC和LCD。
*            进入Active-Halt模式，是通过halt语句。
*            退出是通过外部中断、RTC和复位。
*
*            实现如下功能：（为了保证通信，消息发两次，间隔300毫秒左右）
*                          1、第一次上电时需检测电量，每次报警被触发时，不检测电量，无需报电量低。
*                          2、红外默认--常闭--，即报警信号为低，红外人体告警立即发送报警消息给网关(即外部中断唤醒，报警信号为高)。
*                          3、红外传感器心跳每隔180分钟发送工作状态给网关。
*                          4、正常运行状态下，一天检测两次电量，即每4次心跳，需检测电量，如电量不足，报电量低；
*
*            在该模式下，测试待机电流:1.3uA(单片机+CC1101睡眠模式+KEY断开，无LDO)；
*                        测试待机电流:2.3uA(单片机+CC1101睡眠模式+KEY闭合，无LDO)；上拉电阻为3.3M  3/3.3M = 0.9uA
*                        中断或RTC唤醒时发送电流:约为13mA以上。
*                        以上都用VICTOR VC890C+万用表测量。
* 修改记录 :
*   版本号  日期        作者     说明
*   V1.0    2016-07-29  ShaoPu   正式发布
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

/*定义数据包结构体*/
typedef struct 
{
    uint8_t len;//数据包长度
    uint8_t ctrl;//数据包控制码
    uint8_t type;//设备类型
    uint8_t number[5];//设备序列号
    uint8_t alarm;//报警信息
}TXBUF;

/*模块工作状态*/
typedef enum
{
    SYS_RTC      = 0,
    SYS_EXTI     = 1
}SYS;

/*CC1101发送数据包声明*/
TXBUF txbuf;

/* Private define ------------------------------------------------------------*/

/*设备类型和设备ID保存的Flash起始地址*/
#define start_addr      0xbff0  

/*20分钟启动RTC一次*/
#define HEAT_BTIME	44531	//20min	        //35000 //15min

/*2分钟启动RTC一次*/
//#define HEAT_BTIME	223	//2min

/*180分钟发送心跳包*/
#define HEAT_CNT	8	//20min*9 = 180min---3hour

/*720分钟发送电量包*/
#define BATTERY_CNT	4	//180min*4 = 720min---12hour

/*中断发送时，两个包的间隔时间*/
#define KEY_RECONTIME	5	//135ms

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/*模块工作状态，初始化为待机状态*/
uint8_t work_sta = SYS_RTC;

/*中断唤醒状态下的，延时发送计数*/
uint8_t working_cnt = 0;

/*RTC唤醒状态下的，延时发送计数*/
uint8_t rtc_cnt = 0;

/*唤醒后，当前KEY的电平，在唤醒后检测*/
uint8_t key = 0;

/*唤醒后，上一次KEY的电平，在设备启动时检测*/
volatile uint8_t pre_key = 0;

/*中断唤醒标志位*/
volatile uint8_t ext_flag = 0;

/*RTC休眠时间*/
uint16_t tm = 0;

/*心跳包计数值*/
uint8_t heat_cnt = 0;

/*电量包计数值*/
uint8_t battery_cnt = 0;

/*低电量标志位*/
uint8_t ucBatvot = 0;
/* Private function prototypes -----------------------------------------------*/

/* Private functions ---------------------------------------------------------*/

/*
*********************************************************************************************************
* 函 数 名: TXBUF_Init
* 功能说明: 数据包初始化
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void TXBUF_Init(void)
{
    uint8_t i;
    uint8_t * p;
    
    /*数据包长度:8个字节*/
    txbuf.len = 0x08;
    
    /*数据包控制码*/
    txbuf.ctrl = 0x00;

    /*从Flash中读取设备类型和设备ID*/
    p = (uint8_t *)&(txbuf.type);
    for(i = 0;i<6;i++)
            *(p+i) = FLASH_ReadByte(start_addr+i);
    
    /*设备报警信息*/
    txbuf.alarm = 0x00;
}

/*
*********************************************************************************************************
* 函 数 名: CLK_Config
* 功能说明: 配置时钟
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void CLK_Config(void)  
{
    /*使能时钟切换*/
    CLK_SYSCLKSourceSwitchCmd(ENABLE);
    
    /*选择内部高速时钟作为时钟源*/
    CLK_SYSCLKSourceConfig(CLK_SYSCLKSource_HSI);
    
    /* 设置系统时钟分频: 16*/
    CLK_SYSCLKDivConfig(CLK_SYSCLKDiv_16);
    
    /* 等待时钟稳定*/
    while (CLK_GetSYSCLKSource() != CLK_SYSCLKSource_HSI)
    {     
    }
}

/*
*********************************************************************************************************
* 函 数 名: RTC_Config
* 功能说明: 配置RTC
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void RTC_Config(void)
{
    CLK_LSICmd(ENABLE);
    
    /*等待内部时钟做好准备*/
    while (CLK_GetFlagStatus(CLK_FLAG_LSIRDY) == RESET);
    
    /*开启RTC功能模块的时钟*/
    CLK_PeripheralClockConfig(CLK_Peripheral_RTC, ENABLE);
    
    /*配置RTC时钟，使用内部低速时钟38K，64分频 38000/64 = 593.75 */
    CLK_RTCClockConfig(CLK_RTCCLKSource_LSI, CLK_RTCCLKDiv_64);
    
    /*the wake up Unit must be disabled (if enabled) using RTC_WakeUpCmd(Disable).*/
    RTC_WakeUpCmd(DISABLE);
    
    /*Configures the RTC wakeup timer_step:38k/64/16=37.109375KHz t=27ms */
    RTC_WakeUpClockConfig(RTC_WakeUpClock_RTCCLK_Div16);
       
    /*设置RTC中断优先级*/
    ITC_SetSoftwarePriority(RTC_IRQn, ITC_PriorityLevel_3);
          
    /*使能RTC中断*/
    RTC_ITConfig(RTC_IT_WUT, ENABLE);
         
}

/*
*********************************************************************************************************
* 函 数 名: mcu_sleep
* 功能说明: 设置RTC唤醒时间
* 形    参: cnt:时间值(0-65535)       cnt*27ms
* 返 回 值: 无
*********************************************************************************************************
*/
static void mcu_sleep(uint16_t cnt)
{
    /*设置RTC计数器*/
    RTC_SetWakeUpCounter(cnt);
    
    /*使能RTC唤醒*/
    RTC_WakeUpCmd(ENABLE);	
}

/*
*********************************************************************************************************
* 函 数 名: GPIO_LowPower_Config
* 功能说明: 设置低功耗模式下的GPIO---STM8L151G4
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void GPIO_LowPower_Config(void) 
{
  
    GPIO_DeInit(GPIOA);
    GPIO_DeInit(GPIOB);
    GPIO_DeInit(GPIOC);
    GPIO_DeInit(GPIOD); 
    
    /*将不用的IO全部设置成 GPIO_Mode_Out_OD_Low_Slow*/
    GPIO_Init(GPIOA,  GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5, GPIO_Mode_Out_OD_Low_Slow);
    GPIO_Init(GPIOB,  GPIO_Pin_All, GPIO_Mode_Out_OD_Low_Slow);
    GPIO_Init(GPIOC,  GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|GPIO_Pin_6, GPIO_Mode_Out_OD_Low_Slow);	
    GPIO_Init(GPIOD,  GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|GPIO_Pin_3|GPIO_Pin_4, GPIO_Mode_Out_OD_Low_Slow);    

    /*使能在UltraLowPower模式的快速唤醒*/
    PWR_FastWakeUpCmd(ENABLE);
    
    /*使能UltraLowPower模式*/
    PWR_UltraLowPowerCmd(ENABLE); 
}

/*
*********************************************************************************************************
* 函 数 名: EXTI_Init
* 功能说明: 初始化外部中断---GPIOB2
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void EXTI_Init(void)
{
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_In_FL_IT);
    EXTI_SetPinSensitivity(EXTI_Pin_2, EXTI_Trigger_Rising_Falling);
}

/*
*********************************************************************************************************
* 函 数 名: EXTI_UnInit
* 功能说明: 撤销外部中断，GPIO变为低功耗模式---GPIOB2
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
static void EXTI_UnInit(void)
{
    EXTI_DeInit();
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_Out_OD_Low_Slow);
}

/*
*********************************************************************************************************
* 函 数 名: ext_handle
* 功能说明: 中断调用函数
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
void ext_handle(void)
{
    /*中断标志位置 1*/
    ext_flag = 1;

    EXTI_UnInit();
}

/*
*********************************************************************************************************
* 函 数 名: CC1101_Init
* 功能说明: CC1101初始化
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
void CC1101_Init(void)
{
    /*SPI初始化*/
    SPI_Initial();
    
    /*CC1101初始化*/
    CC1101Init();
    
    /*CC1101清除发送数据*/
    CC1101ClrTXBuff();
    
    /*设置CC1101为发送模式*/
    CC1101SetTRMode(TX_MODE);
    
    /*CC1101进入低功耗模式*/
    CC1101SetIdle();		
    CC1101Sleep();
    CC_CSN_HIGH();
}

/*
*********************************************************************************************************
* 函 数 名: op
* 功能说明: CC1101发送数据包函数
* 形    参: key：KEY电平信息
*           flag：电量信息
* 返 回 值: 无
*********************************************************************************************************
*/
void op(uint8_t key,uint8_t flag)
{
    /*数据包报警信息清零*/
    txbuf.alarm = 0x00;
    
    /*初始化SPI和CC1101 清除发送数据包，设置CC1101为发送模式*/
    SPI_Initial();
    CC1101Init();
    CC1101ClrTXBuff();
    CC1101SetTRMode(TX_MODE);			
 
    /*判断KEY电平信息，如果为1，发送报警信息
                       如果为0，发送正常信息
    */
    if(key!=0)
        txbuf.alarm = 0xf0;
    else
        txbuf.alarm = 0x00;

    /*判断电量信息，如果低电量，即Flag为1，发送低电量信息*/
    if(flag != 0)
        txbuf.alarm |= 0x08;
 
    /*CC1101发送数据包，长度为9*/ 
    CC1101SendPacket((INT8U *)&txbuf,9, ADDRESS_CHECK);
 
    /*短暂延时*/ 
    DelayT_ms(10);
    
    /*发送完成后，CC1101进入低功耗模式*/
    CC1101SetIdle();		
    CC1101Sleep();
    CC_CSN_HIGH();
}

/*
*********************************************************************************************************
* 函 数 名: run
* 功能说明: RTC或中断唤醒处理函数
* 形    参: 无
* 返 回 值: 无
*********************************************************************************************************
*/
void run(void)
{
    /*关闭全局中断*/
    disableInterrupts();
    
    /*RTC唤醒关闭*/
    RTC_WakeUpCmd(DISABLE);
    
    /*唤醒后，将KEY配置成无中断的上拉输入，读取电平*/
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_In_PU_No_IT);
    
    /*读取唤醒后的当前KEY电平*/
    key = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2);
    
    /*判断是否是外部中断唤醒
	  如果是，状态量直接进入中断唤醒的工作状态
    */
    if(ext_flag)
    {
        heat_cnt = 0;//心跳包计数器重置
        ext_flag = 0;//中断标志位重置
        work_sta = SYS_EXTI;//工作状态为外部唤醒状态
        working_cnt = 0;//延时发送计数器重置
    }
    
    /*判断KEY的值是否与上一次相同
      如果不相同，说明是中断唤醒，状态量进入工作状态，保存当前key值，然后CC1101发送数据包一次
      相同的情况，说明RTC唤醒的情况下，无需处理
    */
    if(key!=pre_key)
    {
        heat_cnt = 0;//心跳包计数器重置
        working_cnt = 0;//中断标志位重置	
        work_sta = SYS_EXTI;//工作状态为外部唤醒状态
        pre_key = key;//保存当前电平
        op(key,0);//第一次发送数据包
    }
    
    /*判断唤醒后的工作状态*/
    switch (work_sta)
    {
        /*RTC唤醒*/
        case SYS_RTC:
            /*设置RTC唤醒时间*/
            tm = HEAT_BTIME; //休眠20分钟 
            
            /*心跳包计数器判断是否到达180分钟*/
            if(heat_cnt < HEAT_CNT)
            {
                heat_cnt ++;
            }
            else
            { 
                /*进入RTC唤醒后，等待275ms左右 发送第1个数据包*/
                if(rtc_cnt == 0)
                {                    
                    rtc_cnt = 1;
                    
                    ucBatvot = 0;
                    /*低电量计数器计数，判断是否是到达720分钟
                      如果是，发送带有电量信息的数据包
                      如果不是，正常发送数据包
                    */
                    battery_cnt++;
                    if(battery_cnt>=BATTERY_CNT)
                    {
                        battery_cnt = 0;
                        ucBatvot = GET_Batvot();
                    }
                    
                    op(key,ucBatvot);//第1次发送数据包                   
                    //休眠270ms
                    tm = KEY_RECONTIME;
                }
                else
                {                    
                    rtc_cnt = 0;//延时发送计数器重置
                    
                    /*180min一次真正的心跳到达，发送数据包*/
                    heat_cnt = 0;
                                       
                    op(key,ucBatvot);                
                }
            }
        break;
        
        /*中断唤醒*/
        case SYS_EXTI:
          
            /*进入中断唤醒后，等待275ms左右 发送第二个数据包*/
            if(working_cnt == 0)
            {
                working_cnt = 1;
                //休眠135ms
                tm = KEY_RECONTIME;
            }
            else
            {
                working_cnt = 0;//延时发送计数器重置
                work_sta = SYS_RTC;//工作状态切换为RTC唤醒状态
                
                op(key,0);//第二次发送数据包
                
               /*设置RTC唤醒时间*/
                tm = HEAT_BTIME; //休眠20分钟
                //无线发送，并进入IDLE模式
            }		
         break;
    }
    
    /*重新配置KEY为中断模式*/
    EXTI_Init();
    
    /*重新设置RTC唤醒时间，使能全局中断，进入active halt模式*/
    mcu_sleep(tm);
    enableInterrupts();		
    halt();
}

void main(void)
{  
  
    DelayT_ms(500); 
    
    disableInterrupts();
    
    /*数据包初始化*/
    TXBUF_Init();
    /*设置时钟主频*/ 
//    CLK_Config(); //打不打开在睡眠醒来时电流值影响不大，都是0.735mA
    /*获取电池电量状态*/    
    ucBatvot = GET_Batvot();
    
    /*上电发送带电量的数据包，发送两次*/  
    op(key,ucBatvot);
    DelayT_ms(50);     
    op(key,ucBatvot); 
    DelayT_ms(50);
    op(key,ucBatvot);
    
    /*将KEY配置成无中断的上拉输入，读取电平，将电平保存到pre_key中*/
    GPIO_Init(GPIOB,  GPIO_Pin_2, GPIO_Mode_In_PU_No_IT);
    pre_key = GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2);
    
    /*配置系统时钟在Halt或Active-Halt模式下*/
    CLK_HaltConfig(CLK_Halt_FastWakeup,ENABLE); 
    
    /*低功耗模式下GPIO初始化*/
    GPIO_LowPower_Config();
    
    /*CC1101初始化，RTC初始化，中断初始化，设置RTC唤醒时间，使能全局中断，进入activ halt模式*/
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
