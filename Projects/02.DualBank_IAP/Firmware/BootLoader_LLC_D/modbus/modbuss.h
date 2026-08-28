/*
 * modbuss.h
 *
 *  Created on: 2024年3月28日
 *      Author: Liang Jinfeng
 */

#ifndef MODBUSS_H
#define MODBUSS_H

#include "modbus.h"

/*
 * 初始化
 */
void modbuss_init(uchar addr);

/*
 * 设置通信超时计数值
 */
void modbuss_set_timeout(ulong timeout);

/*
 * 设置发送数据回调函数
 */
void modbuss_set_send_callback(void (*callback)(const void *buff,ushort size));

/*
 * 设置读寄存器回调函数
 */
void modbuss_set_read_reg_callback(void (*callback)(modbus_reg_cmd_t *pkt));

/*
 * 设置写寄存器回调函数
 */
void modbuss_set_write_reg_callback(void (*callback)(modbus_reg_data_t *pkt));

/*
 * 设置IAP命令回调函数
 */
void modbuss_set_iap_callback(void (*callback)(modbus_iap_t *pkt));

/*
 * 回应读寄存器数据
 */
void modbuss_ack_read_reg(ushort reg,ushort num,const void *data,ushort size);

/*
 * 回应写寄存器数据
 */
void modbuss_ack_write_reg(ushort reg,ushort num);

/*
 * 发送IAP命令
 */
void modbuss_send_iap(uchar *cmd,const void *data,ushort size);

/*
 * 接收一个字节
 */
void modbuss_recv_byte(uchar data);

/*
 * MODBUS从机任务
 */
void modbuss_task();

#endif /* MODBUSS_H */
