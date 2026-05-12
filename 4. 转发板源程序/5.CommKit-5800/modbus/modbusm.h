/*
 * modbusm.h
 *
 *  Created on: 2024年4月4日
 *      Author: Liang Jinfeng
 */

#ifndef MODBUSM_H
#define MODBUSM_H

#include "modbus.h"

/*
 * 初始化
 */
void modbusm_init();

/*
 * 设置通信超时计数值
 */
void modbusm_set_timeout(ulong timeout);

/*
 * 设置发送数据回调函数
 */
void modbusm_set_send_callback(void (*callback)(const void *buff,ushort size));

/*
 * 设置读寄存器ACK回调函数
 */
void modbusm_set_read_reg_ack_callback(void (*callback)(modbus_reg_data_t *pkt));

/*
 * 设置写寄存器ACK回调函数
 */
void modbusm_set_write_reg_ack_callback(void (*callback)(modbus_reg_cmd_t *pkt));

/*
 * 设置IAP命令回调函数
 */
void modbusm_set_iap_cmd_ack_callback(void (*callback)(modbus_iap_t *pkt));

/*
 * 读寄存器
 */
void modbusm_read_reg(uchar addr,ushort reg,ushort num);

/*
 * 写寄存器
 */
void modbusm_write_reg(uchar addr,ushort reg,ushort num,const void *data,ushort size);

/*
 * 发送IAP命令
 */
void modbusm_iap_cmd(uchar addr,uchar *cmd,const void *data,ushort size);

/*
 * 接收一个字节
 */
void modbusm_recv_byte(uchar data);

/*
 * MODBUS主机任务
 */
void modbusm_task();

#endif /* MODBUSM_H */
