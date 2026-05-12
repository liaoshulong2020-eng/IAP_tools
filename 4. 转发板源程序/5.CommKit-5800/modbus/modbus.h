/*
 * modbus.h
 *
 *  Created on: 2024年4月3日
 *      Author: Liang Jinfeng
 */

#ifndef MODBUS_H
#define MODBUS_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/

//寄存器操作最大载荷数据量
#define MODBUS_MAX_REG_PAYLOAD_SIZE		256

//ModBus数据域最大长度
#define MODBUS_MAX_DATA_SIZE			(MODBUS_MAX_REG_PAYLOAD_SIZE+12)

/*
 * 支持的功能码
 */
#define FNO_RD_REG						0x03 //读寄存器
#define FNO_WR_REG						0x10 //写寄存器
#define FNO_IAP							0x41 //IAP命令

/*******************************************************************************
 * 数据结构
 ******************************************************************************/

/*
 * modbus数据帧（应用数据单元 ADU）
 */
typedef struct
{
	uchar addr; //地址
	uchar fno; //功能码
	uchar data[MODBUS_MAX_DATA_SIZE]; //数据域，包括2字节CRC校验码
}__attribute__((packed))modbus_pkt_t;

/*
 * 寄存器命令帧
 * 主机：读寄存器请求帧
 * 从机：写寄存器应答帧
 */
typedef struct
{
	uchar addr; //地址
	uchar fno; //功能码
	ushort reg; //寄存器起始地址
	ushort num; //寄存器数量
	ushort crc;
}__attribute__((packed))modbus_reg_cmd_t;

/*
 * 寄存器数据帧
 * 主机：写寄存器请求帧
 * 从机：读寄存器应答帧
 */
typedef struct
{
	uchar addr; //地址
	uchar fno; //功能码
	ushort reg; //寄存器起始地址
	ushort num; //寄存器数量
	ushort size; //寄存器数据总长度
	uchar data[MODBUS_MAX_REG_PAYLOAD_SIZE+2]; //数据域，包括2字节CRC校验码
}__attribute__((packed))modbus_reg_data_t;

/*
 * IAP数据帧
 */
typedef struct
{
	uchar addr; //地址
	uchar fno; //功能码
	uchar cmd[8]; //IAP命令体
	ushort size; //IAP数据长度
	uchar data[MODBUS_MAX_REG_PAYLOAD_SIZE+2]; //数据域，包括2字节CRC校验码
}__attribute__((packed))modbus_iap_t;

#endif // MODBUS_H
