/*
*********************************************************************************************************
*
* 模块名称 : 数据类型定义模块
* 文件名称 : MyTypeDef.c
* 版    本 : V1.0
* 说    明 : 通用数据类型定义
*
* 修改记录 :
*   版本号  日期        作者     说明
*   V1.0    2016-07-28  ShaoPu   正式发布
*
*********************************************************************************************************
*/
#ifndef _MYTYPEDEF_H_
#define _MYTYPEDEF_H_

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"

/*Data type definations*/  
typedef unsigned char  INT8U;
typedef signed   char  INT8S;
typedef unsigned int   INT16U;
typedef signed   int   INT16S;
typedef unsigned long  INT32U;
typedef signed   long  INT32S;
typedef enum
{
	BOOL_FALSE = 0,
	BOOL_TRUE  = 1
}BOOLEAN;

/*Bit field operations*/
//#define SetBit( Byte, Bit )  ( Byte ) |= ( 1<<( Bit ) )
//#define ClrBit( Byte, Bit )  ( Byte ) &= ~( 1<<( Bit ) )
#define GetBit( Byte, Bit )  ( ( Byte ) & ( 1<<( Bit ) ) )
#define ComBit( Byte, Bit )  ( Bytes ) ^= ( 1<<( Bit ) )
#define SetBits( Byte, Bits ) ( Byte ) |= ( Bits )
#define ClrBits( Byte, Bits ) ( Byte ) &= ~( Bits )
#define GetBits( Byte, Bits ) ( ( Byte ) & ( Bits ) )
#define ComBits( Byte, Bits ) ( Byte ) ^= ( Bits )

/*Bit map for fast calculation*/
#define BitMap( x )   ( 1<<( x ) )
/*Get MAX or MIN value*/
#define GetMax( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x1 ) : ( x2 ) )
#define GetMin( x1, x2 ) ( ( x1 ) > ( x2 ) ? ( x2 ) : ( x1 ) )

#endif//_MYTYPEDEF_H_

/***********************************************(END OF FILE) *******************************************/
