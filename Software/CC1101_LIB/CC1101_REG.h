/*
*********************************************************************************************************
*
*	模块名称 : CC1101寄存器配置模块
*	文件名称 : CC1101_REG.h
*	版    本 : V1.0
*	说    明 : 头文件
*	修改记录 :
*		版本号  日期        作者     说明
*		V1.0    2016-07-26  ShaoPu  正式发布
*********************************************************************************************************
*/
#ifndef _CC1101_REH_H_
#define _CC1101_REH_H_

//CC1101寄存器定义  
typedef enum  
{  
    //可读写的寄存器  
    CC1101_IOCFG2       =   0x00,   //GDO2输出脚配置  
    CC1101_IOCFG1       =   0x01,   //GDO1输出脚配置  
    CC1101_IOCFG0       =   0x02,   //GDO0输出脚配置  
    CC1101_FIFOTHR      =   0x03,   //RX FIFO和TX FIFO门限  
    CC1101_SYNC1        =   0x04,   //同步词汇，高字节  
    CC1101_SYNC0        =   0x05,   //同步词汇，低字节  
    CC1101_PKTLEN       =   0x06,   //数据包长度  
    CC1101_PKTCTRL1     =   0x07,   //数据包自动控制  
    CC1101_PKTCTRL0     =   0x08,   //数据包自动控制  
    CC1101_ADDR         =   0x09,   //设备地址  
    CC1101_CHANNR       =   0x0a,   //信道数  
    CC1101_FSCTRL1      =   0x0b,   //频率合成器控制，高字节  
    CC1101_FSCTRL0      =   0x0c,   //频率合成器控制，低字节  
    CC1101_FREQ2        =   0x0d,   //频率控制词汇，高字节  
    CC1101_FREQ1        =   0x0e,   //频率控制词汇，中间字节  
    CC1101_FREQ0        =   0x0f,   //频率控制词汇，低字节  
    CC1101_MDMCFG4      =   0x10,   //调制器配置  
    CC1101_MDMCFG3      =   0x11,   //调制器配置  
    CC1101_MDMCFG2      =   0x12,   //调制器配置  
    CC1101_MDMCFG1      =   0x13,   //调制器配置  
    CC1101_MDMCFG0      =   0x14,   //调制器配置  
    CC1101_DEVIATN      =   0x15,   //调制器背离设置  
    CC1101_MCSM2        =   0x16,   //主通信控制状态机配置  
    CC1101_MCSM1        =   0x17,   //主通信控制状态机配置  
    CC1101_MCSM0        =   0x18,   //主通信控制状态机配置  
    CC1101_FOCCFG       =   0x19,   //频率偏移补偿配置  
    CC1101_BSCFG        =   0x1A,   //位同步配置  
    CC1101_AGCTRL2      =   0x1B,   //AGC控制  
    CC1101_AGCTRL1      =   0x1C,   //AGC控制  
    CC1101_AGCTRL0      =   0x1D,   //AGC控制  
    CC1101_WOREVT1      =   0x1E,   //高字节时间0暂停  
    CC1101_WOREVT0      =   0x1F,   //低字节时间0暂停  
    CC1101_WORCTRL      =   0x20,   //电磁波激活控制  
    CC1101_FREND1       =   0x21,   //前末端RX配置  
    CC1101_FREND0       =   0x22,   //前末端TX配置  
    CC1101_FSCAL3       =   0x23,   //频率合成器校准  
    CC1101_FSCAL2       =   0x24,   //频率合成器校准  
    CC1101_FSCAL1       =   0x25,   //频率合成器校准  
    CC1101_FSCAL0       =   0x26,   //频率合成器校准  
    CC1101_RCCTRL1      =   0x27,   //RC振荡器配置  
    CC1101_RCCTRL0      =   0x28,   //RC振荡器配置  
    CC1101_FSTEST       =   0x29,   //频率合成器校准控制  
    CC1101_PTEST        =   0x2A,   //产品测试  
    CC1101_AGCTEST      =   0x2B,   //AGC测试  
    CC1101_TEST2        =   0x2C,   //不同的测试设置  
    CC1101_TEST1        =   0x2D,   //不同的测试设置  
    CC1101_TEST0        =   0x2E,   //不同的测试设置  
    //只读的状态寄存器,如果写入将导致命令滤波 
    CC1101_PARTNUM      =   0x30,   //CC2550的组成部分数目  
    CC1101_VERSION      =   0x31,   //当前版本数  
    CC1101_FREQEST      =   0x32,   //频率偏移估计  
    CC1101_LQI          =   0x33,   //连接质量的解调器估计  
    CC1101_RSSI         =   0x34,   //接收信号强度指示  
    CC1101_MARCSTATE    =   0x35,   //控制状态机状态  
    CC1101_WORTIME1     =   0x36,   //WOR计时器高字节  
    CC1101_WORTIME0     =   0x37,   //WOR计时器低字节  
    CC1101_PKTSTATUS    =   0x38,   //当前GDOx状态和数据包状态  
    CC1101_VCOVCDAC     =   0x39,   //PLL校准模块的当前设定  
    CC1101_TXBYTES      =   0x3A,   //TX FIFO中的下溢和比特数  
    CC1101_RXBYTES      =   0x3B,   //RX FIFO中的下溢和比特数  
    //功率控制
    CC1101_PATABLE      =   0x3E,   //功率控制
    CC1101_TXFIFO       =   0x3F,   //单字节写 TX FIFO  
    CC1101_RXFIFO       =   0x3F    //单字节写 RX FIFO   
}CC1101_REG_TYPE;  

//CC1101 命令  
//以写的方式单直接访问将触发响应的命令  
typedef enum  
{  
    CC1101_SRES     =   0x30,   //复位芯片 
    CC1101_SFSTXON  =   0x31,   //开启和校准频率合成器（若MCSM0.FSAUTOCAL=1）  
    CC1101_SXOFF    =   0x32,   //关闭晶体振荡器  
    CC1101_SCAL     =   0x33,   //校准频率合成器并关断（开启快速启动）。在不设置手动校准模式（MCSM0.FS_AUTOCAL=0）的情况下，SCAL能从空闲模式滤波。  
    CC1101_SRX      =   0x34,   //启用RX。若上一状态为空闲且MCSM0.FS_AUTOCAL=1则首先运行校准。  
    CC1101_STX      =   0x35,   //空闲状态：启用TX。若MCSM0.FS_AUTOCAL=1首先运行校准。若在RX状态且CCA启用：若信道为空则进入TX  
    CC1101_SIDLE    =   0x36,   //退出RX/TX,关断频率合成器并退出电磁波激活模式若可用  
    CC1101_SAFC     =   0x37,   //运行22.1节列出的频率合成器的AFC调节  
    CC1101_SWOR     =   0x38,   //运行27.5节描述的自动RX选举序列（电磁波激活）  
    CC1101_SPWD     =   0x39,   //当CSn为高时进入功率降低模式。(CC1101低功耗模式)  
    CC1101_SFRX     =   0x3A,   //清除RX FIFO缓冲  
    CC1101_SFTX     =   0x3B,   //清除TX FIFO缓冲  
    CC1101_SWORRST  =   0x3C,   //重新设置真实时间时钟  
    CC1101_SNOP     =   0x3D    //无操作。可能用来为更简单的软件将滤波命令变为2字节。  
}CC1101_CMD_TYPE;  

#define WRITE_BURST     	0x40	//连续写入
#define READ_SINGLE     	0x80	//读
#define READ_BURST      	0xC0	//连续读
#define BYTES_IN_RXFIFO         0x7F  	//接收缓冲区的有效字节数
#define CRC_OK                  0x80 	//CRC校验通过位标志


#endif //_CC1101_REH_H_
/***********************************************(END OF FILE) *******************************************/
