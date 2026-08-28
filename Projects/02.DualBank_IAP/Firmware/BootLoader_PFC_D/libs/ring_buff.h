/*
 * ring_buff.h
 *
 *  Created on: 2024年3月21日
 *      Author: Liang Jinfeng
 */

#ifndef RING_BUFF_H
#define RING_BUFF_H

#include "main.h"

/*******************************************************************************
 * 参数配置
 ******************************************************************************/

//缓冲区大小
#define RING_BUFF_SIZE			288

/*******************************************************************************
 * 数据结构
 ******************************************************************************/

/*
 * 环形缓冲区
 */
typedef struct
{
	uchar buff[RING_BUFF_SIZE]; //缓冲区
	ushort use_begin; //已用区起始位置
	ushort free_begin; //未用区起始位置
	ushort free_size; //可用大小
	ushort use_size; //已用大小
}ring_buff_t;

/*******************************************************************************
 * 接口函数
 ******************************************************************************/

/*
 * 初始化
 */
void ring_buff_init(ring_buff_t *ring);

/*
 * 保存数据
 */
void ring_buff_save_data(ring_buff_t *ring,const void *data,ushort size);

/*
 * 提取一个字节
 */
bool ring_buff_take_byte(ring_buff_t *ring,uchar *byte);

/*
 * 缓冲区是否为空
 */
bool ring_buff_is_empty(ring_buff_t *ring);

/*
 * 缓冲区是否已满
 */
bool ring_buff_is_full(ring_buff_t *ring);

#endif /* RING_BUFF_H */
