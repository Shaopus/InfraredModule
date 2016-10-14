/*
*********************************************************************************************************
*
* ģ������ : CC1101����ģ��
* �ļ����� : CC1101.c
* ��    �� : V1.0
* ˵    �� : CC1101������ģ��
*
* �޸ļ�¼ :
*   �汾��  ����        ����     ˵��
*   V1.0    2016-07-26  ShaoPu   ��ʽ����
*
*********************************************************************************************************
*/
#include "CC1101.H"

//10, 7, 5, 0, -5, -10, -15, -20, dbm output power, 0x12 == -30dbm
const INT8U PaTabel[] = { 0xc0, 0xC8, 0x84, 0x60, 0x68, 0x34, 0x1D, 0x0E};

// Sync word qualifier mode = 30/32 sync word bits detected 
// CRC autoflush = false 
// Channel spacing = 199.951172 
// Data format = Normal mode 
// Data rate = 2.00224 
// RX filter BW = 58.035714 
// PA ramping = false 
// Preamble count = 4 
// Whitening = false 
// Address config = No address check 
// Carrier frequency = 400.199890 
// Device address = 0 
// TX power = 10 
// Manchester enable = false 
// CRC enable = true 
// Deviation = 5.157471 
// Packet length mode = Variable packet length mode. Packet length configured by the first byte after sync word 
// Packet length = 255 
// Modulation format = GFSK 
// Base frequency = 399.999939 
// Modulated = true 
// Channel number = 1 
// PA table 
#define PA_TABLE {0xc2,0x00,0x00,0x00,0x00,0x00,0x00,0x00,}

#if 0
static const INT8U CC1101InitData[22][2]= 
{
  {CC1101_IOCFG0,      0x06},
  {CC1101_FIFOTHR,     0x47},
  {CC1101_PKTCTRL0,    0x05},
  {CC1101_CHANNR,      0x01},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x0F},
  {CC1101_FREQ1,       0x62},
  {CC1101_FREQ0,       0x76},
  {CC1101_MDMCFG4,     0xF6},
  {CC1101_MDMCFG3,     0x43},
  {CC1101_MDMCFG2,     0x13},
  {CC1101_DEVIATN,     0x15},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_MCSM1,       0x3B},
};
#else

static const INT8U CC1101InitData[23][2]= 
{
  {CC1101_IOCFG0,      0x06},
  {CC1101_FIFOTHR,     0x47},
  {CC1101_PKTCTRL0,    0x05},
  {CC1101_CHANNR,      0x01},
  {CC1101_FSCTRL1,     0x06},
  {CC1101_FREQ2,       0x10},
  {CC1101_FREQ1,       0xEC},
  {CC1101_FREQ0,       0x4E},
  {CC1101_MDMCFG4,     0xF6},
  {CC1101_MDMCFG3,     0x43},
  {CC1101_MDMCFG2,     0x13},
  {CC1101_DEVIATN,     0x15},
  {CC1101_MCSM0,       0x18},
  {CC1101_FOCCFG,      0x16},
  {CC1101_WORCTRL,     0xFB},
  {CC1101_FSCAL3,      0xE9},
  {CC1101_FSCAL2,      0x2A},
  {CC1101_FSCAL1,      0x00},
  {CC1101_FSCAL0,      0x1F},
  {CC1101_TEST2,       0x81},
  {CC1101_TEST1,       0x35},
  {CC1101_TEST0,       0x09},
  {CC1101_MCSM1,       0x3B},		
  //MCSM1���ֵSmartRF Stdioû�����ɣ���Ҫ�������
};
#endif


/*
*********************************************************************************************************
* �� �� ��: CC1101ReadReg
* ����˵��: ��ȡCC1101ͨ�üĴ���һ���ֽ�����
* ��    ��: addr:�Ĵ�����ַ,��:CC1101_REG.H
* �� �� ֵ: i:�Ĵ�����ֵ 
*********************************************************************************************************
*/
static INT8U CC1101ReadReg( INT8U addr )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | READ_SINGLE);
    i = SPI_ExchangeByte( 0xFF );
    CC_CSN_HIGH( );
    return i;
}

/*
*********************************************************************************************************
* �� �� ��: CC1101ReadMultiReg
* ����˵��: ��ȡCC1101ͨ�üĴ�������ֽ�����
* ��    ��: addr:�Ĵ�����ַ,��:CC1101_REG_TYPE
*           buff:��ȡ��������
*           size:��Ҫ��ȡ���ݴ�С
* �� �� ֵ: �� 
*********************************************************************************************************
*/
static void CC1101ReadMultiReg( INT8U addr, INT8U *buff, INT8U size )
{
    INT8U i, j;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | READ_BURST);
    for( i = 0; i < size; i ++ )
    {
        for( j = 0; j < 20; j ++ );
        *( buff + i ) = SPI_ExchangeByte( 0xFF );
    }
    CC_CSN_HIGH( );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101ReadStatus
* ����˵��: ��ȡCC1101״̬�Ĵ���(0x30~0x3D)
* ��    ��: addr:�Ĵ�����ַ,��:CC1101_CMD_TYPE
* �� �� ֵ: i:״̬�Ĵ�����ǰֵ
*********************************************************************************************************
*/
static INT8U CC1101ReadStatus( INT8U addr )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | READ_BURST);
    i = SPI_ExchangeByte( 0xFF );
    CC_CSN_HIGH( );
    return i;
}

/*
*********************************************************************************************************
* �� �� ��: CC1101WriteCmd
* ����˵��: д�뵥�ֽ�����
* ��    ��: command:д��CC1101���� 
* �� �� ֵ: ��
*********************************************************************************************************
*/
static void CC1101WriteCmd( INT8U command )
{
    CC_CSN_LOW( );
    SPI_ExchangeByte( command );
    CC_CSN_HIGH( );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101WriteReg
* ����˵��: д��CC1101ͨ�üĴ���һ���ֽ�����
* ��    ��: addr:�Ĵ�����ַ,��:CC1101_REG.H
*           value:��Ҫд�������
* �� �� ֵ: i:�Ĵ�����ֵ 
*********************************************************************************************************
*/
static void CC1101WriteReg( INT8U addr, INT8U value )
{
    CC_CSN_LOW( );
    SPI_ExchangeByte(addr);
    SPI_ExchangeByte(value);
    CC_CSN_HIGH( );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101WriteMultiReg
* ����˵��: д��CC1101ͨ�üĴ�������ֽ�����
* ��    ��: addr:�Ĵ�����ַ,��:CC1101_REG.H
*           buff:��Ҫд��Ļ�������
*           size:д�����ݵĴ�С
* �� �� ֵ: �� 
*********************************************************************************************************
*/
static void CC1101WriteMultiReg( INT8U addr, INT8U *buff, INT8U size )
{
    INT8U i;
    CC_CSN_LOW( );
    SPI_ExchangeByte( addr | WRITE_BURST );
    for( i = 0; i < size; i ++ )
    {
        SPI_ExchangeByte( *( buff + i ) );
    }
    CC_CSN_HIGH( );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101Sleep
* ����˵��: ����CC1101����˯��ģʽ
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101Sleep(void)
{
    CC1101WriteCmd(CC1101_SPWD);
}

/*
*********************************************************************************************************
* �� �� ��: CC1101SetIdle
* ����˵��: ����CC1101�������ģʽ
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101SetIdle( void )
{
    CC1101WriteCmd(CC1101_SIDLE);
}

/*
*********************************************************************************************************
* �� �� ��: CC1101SetTRMode
* ����˵��: ����CC1101ģʽ
* ��    ��: mode������ģʽ���߽���ģʽ
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101SetTRMode( TRMODE mode )
{
    if( mode == TX_MODE )
    {
        CC1101WriteReg(CC1101_IOCFG0,0x46);
        CC1101WriteCmd( CC1101_STX );
    }
    else if( mode == RX_MODE )
    {
        CC1101WriteReg(CC1101_IOCFG0,0x46);
        CC1101WriteCmd( CC1101_SRX );
    }
}

/*
*********************************************************************************************************
* �� �� ��: CC1101Reset
* ����˵��: ����CC1101
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101Reset( void )
{
    INT8U x;

    CC_CSN_HIGH( );
    CC_CSN_LOW( );
    CC_CSN_HIGH( );
    for( x = 0; x < 100; x ++ );
    CC1101WriteCmd( CC1101_SRES );
}


/*
*********************************************************************************************************
* �� �� ��: CC1101ClrTXBuff
* ����˵��: ������͵�FIFO����
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101ClrTXBuff( void )
{
    CC1101SetIdle();//MUST BE IDLE MODE
    CC1101WriteCmd( CC1101_SFTX );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101ClrRXBuff
* ����˵��: ������ܵ�FIFO����
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101ClrRXBuff( void )
{
    CC1101SetIdle();//MUST BE IDLE MODE
    CC1101WriteCmd( CC1101_SFRX );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101SendPacket
* ����˵��: CC1101�������ݰ�---(һ�����64B,��Ϊ�ܵ�FIFO����)
* ��    ��: txbuffer:��Ҫ���͵����ݻ�����ָ��
*           size:�������ݵĴ�С
*           mode:�������ݵ�ģʽ(�㲥���ߵ�ַ���)
* �� �� ֵ: �� 
*********************************************************************************************************
*/
void CC1101SendPacket( INT8U *txbuffer, INT8U size, TX_DATA_MODE mode )
{
    INT8U address;
    if( mode == BROADCAST )             { address = 0; }
    else if( mode == ADDRESS_CHECK )    { address = CC1101ReadReg( CC1101_ADDR ); }

    CC1101ClrTXBuff( );
    
    if( ( CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03 ) != 0 )
    {
        CC1101WriteReg( CC1101_TXFIFO, size + 1 );
        CC1101WriteReg( CC1101_TXFIFO, address );
    }
    else
    {
        CC1101WriteReg( CC1101_TXFIFO, size );
    }

    CC1101WriteMultiReg( CC1101_TXFIFO, txbuffer, size );
    CC1101SetTRMode( TX_MODE );
    // Wait for GDO0 to be set -> sync transmitted
    while( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_3 ) != 0 );
    // Wait for GDO0 to be cleared -> end of packet
    while( GPIO_ReadInputDataBit( GPIOB, GPIO_Pin_3 ) == 0 );

    CC1101ClrTXBuff( );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101GetRXCnt
* ����˵��: ���ܵ������ݴ�С
* ��    ��: ��
* �� �� ֵ: �������ݴ�С 
*********************************************************************************************************
*/
INT8U CC1101GetRXCnt( void )
{
    return ( CC1101ReadStatus( CC1101_RXBYTES )  & BYTES_IN_RXFIFO );
}

/*
*********************************************************************************************************
* �� �� ��: CC1101SetAddress
* ����˵��: ����CC1101�ĵ�ַ��
* ��    ��: address:��ַ��
*           AddressMode:��ַģʽ
* �� �� ֵ: �� 
*********************************************************************************************************
*/
void CC1101SetAddress( INT8U address, ADDR_MODE AddressMode)
{
    INT8U btmp = CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03;
    CC1101WriteReg(CC1101_ADDR, address);
    if     ( AddressMode == BROAD_ALL )     {}
    else if( AddressMode == BROAD_NO  )     { btmp |= 0x01; }
    else if( AddressMode == BROAD_0   )     { btmp |= 0x02; }
    else if( AddressMode == BROAD_0AND255 ) { btmp |= 0x03; }   
}

/*
*********************************************************************************************************
* �� �� ��: CC1101SetSYNC
* ����˵��: ����CC1101��ͬ���ֽ�
* ��    ��: sync:ͬ���ֽ�
* �� �� ֵ: �� 
*********************************************************************************************************
*/
void CC1101SetSYNC( INT16U sync )
{
    CC1101WriteReg(CC1101_SYNC1, 0xFF & ( sync>>8 ) );
    CC1101WriteReg(CC1101_SYNC0, 0xFF & sync ); 
}

/*
*********************************************************************************************************
* �� �� ��: CC1101RecPacket
* ����˵��: CC1101�������ݰ�
* ��    ��: rxBuffer:�������ݻ�����ָ��
* �� �� ֵ: pktLen:�������ݳ��ȣ�
*           0:����
*********************************************************************************************************
*/
INT8U CC1101RecPacket( INT8U *rxBuffer )
{
    INT8U status[2];
    INT8U pktLen;
    INT16U x ;

    if ( CC1101GetRXCnt( ) != 0 )                       //����ӵ��ֽ�����Ϊ0
    {
        pktLen = CC1101ReadReg(CC1101_RXFIFO);          //������һ���ֽڣ����ֽ�Ϊ��֡���ݳ���
        if( ( CC1101ReadReg( CC1101_PKTCTRL1 ) & ~0x03 ) != 0 )
        {
            x = CC1101ReadReg(CC1101_RXFIFO);
        }
        if( pktLen == 0 )           { return 0; }       //����
        else                        { pktLen --; }
        CC1101ReadMultiReg(CC1101_RXFIFO, rxBuffer, pktLen); //�������н��յ�������
        CC1101ReadMultiReg(CC1101_RXFIFO, status, 2);   //����CRCУ��λ

        CC1101ClrRXBuff( );                             //������ܻ�����

        if( status[1] & CRC_OK ) {   return pktLen; }   //���У��ɹ������ؽ������ݳ���
        else                     {   return 0; }        //����
    }
    else   {  return 0; }                               //����
}

/*
*********************************************************************************************************
* �� �� ��: CC1101Init
* ����˵��: ��ʼ��CC1101
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void CC1101Init( void )
{
    volatile INT8U i, j;

    CC1101Reset( );//��λCC1101
    
    for( i = 0; i < 23; i++ )
    {
        CC1101WriteReg( CC1101InitData[i][0], CC1101InitData[i][1] );//���ò���
    }
    
    CC1101SetAddress( 0x05, BROAD_0AND255 );//���õ�ַ��
    CC1101SetSYNC( 0x8799 );//ͬ����
    CC1101WriteReg(CC1101_MDMCFG1,   0x72); //���õ��ƽ����

    CC1101WriteMultiReg(CC1101_PATABLE,(INT8U *)PaTabel, 8 );//set patable

    i = CC1101ReadStatus( CC1101_PARTNUM );//for test, must be 0x80
    i = CC1101ReadStatus( CC1101_VERSION );//for test, refer to the datasheet
}

/*
*********************************************************************************************************
* �� �� ��: CC1101WORInit
* ����˵��: ��ʼ��CC1101��WOR����ģʽ
* ��    ��: ��
* �� �� ֵ: ��
*********************************************************************************************************
*/
void  CC1101WORInit( void )
{

    CC1101WriteReg(CC1101_MCSM0,0x18);
    CC1101WriteReg(CC1101_WORCTRL,0x78); //Wake On Radio Control
    CC1101WriteReg(CC1101_MCSM2,0x00);
    CC1101WriteReg(CC1101_WOREVT1,0x8C);
    CC1101WriteReg(CC1101_WOREVT0,0xA0);
	
    CC1101WriteCmd( CC1101_SWORRST );
}

/***********************************************(END OF FILE) *******************************************/
