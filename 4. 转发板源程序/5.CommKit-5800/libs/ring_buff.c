/*
 * ring_buff.c
 *
 *  Created on: 2024年3月21日
 *      Author: Liang Jinfeng
 */

#include "ring_buff.h"

/*
 * 初始化
 */
void ring_buff_init(ring_buff_t *ring)
{
	memset(ring,0,sizeof(ring_buff_t));
	ring->free_size=RING_BUFF_SIZE;
}

/*
 * 保存数据
 */
void ring_buff_save_data(ring_buff_t *ring,const void *data,ushort size)
{
	ushort index,i;

	if(size>ring->free_size)return;
	index=ring->free_begin;
	//复制数据
	for(i=0;i<size;i++)
	{
		ring->buff[index]=((uchar*)data)[i];
		index++;
		if(index>=RING_BUFF_SIZE)index=0;
	}
	ring->free_begin=index;
	ring->free_size-=size;
	ring->use_size+=size;
}

/*
 * 提取一个字节
 */
bool ring_buff_take_byte(ring_buff_t *ring,uchar *byte)
{
	ushort index;

	if(ring->use_size<1)return false;

	index=ring->use_begin;
	//复制数据
	*byte=ring->buff[index];
	index++;
	if(index>=RING_BUFF_SIZE)index=0;
	ring->use_begin=index;
	ring->free_size++;
	ring->use_size--;

	return true;
}

/*
 * 缓冲区是否为空
 */
bool ring_buff_is_empty(ring_buff_t *ring)
{
	if(ring->use_size==0)return true;
	return false;
}

/*
 * 缓冲区是否已满
 */
bool ring_buff_is_full(ring_buff_t *ring)
{
	if(ring->free_size==0)return true;
	return false;
}
