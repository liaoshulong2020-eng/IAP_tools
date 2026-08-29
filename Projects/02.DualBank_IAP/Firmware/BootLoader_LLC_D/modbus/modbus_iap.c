/* LLC Bootloader CAN-to-UART PFC IAP gateway. */
#include "modbus_iap.h"
#include "iap_runtime.h"
#include "modbuss.h"
#include "uart.h"
#include "vars.h"
#include "iap.h"
#include "cans_modbus.h"
#include "crc16.h"
#include "sys_mgr.h"

#define PFC_RESPONSE_TIMEOUT_MS 3500UL
#define PFC_RESET_TIMEOUT_MS 12000UL
#define PFC_GATEWAY_IDLE_MS 60000UL
#define IAP_PACKET_OVERHEAD 14U

typedef enum { PFC_GW_IDLE=0, PFC_GW_WAIT_ACK, PFC_GW_WAIT_RESET } pfc_gateway_state_t;
static bool pfc_forward_mode;
static pfc_gateway_state_t pfc_state;
static ulong pfc_state_started,pfc_last_activity;
static uchar pfc_rx[sizeof(modbus_iap_t)];
static ushort pfc_rx_size,pfc_rx_expected;
static uchar pending_request[sizeof(modbus_iap_t)];
static ushort pending_request_size,pending_command;
static uchar cached_request[sizeof(modbus_iap_t)],cached_ack[sizeof(modbus_iap_t)];
static ushort cached_request_size,cached_ack_size;

static bool packet_equal(const uchar *a,ushort as,const uchar *b,ushort bs)
{ return as==bs && as>0 && memcmp(a,b,as)==0; }

static void pfc_parser_reset(void){pfc_rx_size=0;pfc_rx_expected=0;}

static bool packet_crc_valid(const uchar *data,ushort size)
{
	ushort expected,actual=0;
	if(size<IAP_PACKET_OVERHEAD)return false;
	expected=(ushort)data[size-2]|((ushort)data[size-1]<<8);
	crc16_update(&actual,data,size-2);
	return expected==actual;
}

static ushort build_packet(const modbus_iap_t *pkt,uchar *out)
{
	ushort crc=0,size;
	if(pkt->size>MODBUS_MAX_REG_PAYLOAD_SIZE)return 0;
	out[0]=pkt->addr;out[1]=pkt->fno;
	memmove(out+2,pkt->cmd,8);memmove(out+10,&pkt->size,2);
	memmove(out+12,pkt->data,pkt->size);size=(ushort)(12+pkt->size);
	crc16_update(&crc,out,size);out[size++]=(uchar)crc;out[size++]=(uchar)(crc>>8);
	return size;
}

static void gateway_close(void)
{
	pfc_forward_mode=false;pfc_state=PFC_GW_IDLE;
	pending_request_size=0;pending_command=0;
	cached_request_size=0;cached_ack_size=0;pfc_parser_reset();
}

static void forward_pfc_ack(void)
{
	modbus_iap_t *ack;ushort ack_cmd;
	if(!packet_crc_valid(pfc_rx,pfc_rx_size)){pfc_parser_reset();return;}
	ack=(modbus_iap_t*)pfc_rx;
	ack_cmd=(ushort)ack->cmd[0]|((ushort)ack->cmd[1]<<8);
	if(ack->addr!=MODBUS_PFC_ADDR || ack->fno!=FNO_IAP ||
	   pfc_state!=PFC_GW_WAIT_ACK || ack_cmd!=pending_command)
	{pfc_parser_reset();return;}
	memmove(cached_request,pending_request,pending_request_size);
	cached_request_size=pending_request_size;
	memmove(cached_ack,pfc_rx,pfc_rx_size);cached_ack_size=pfc_rx_size;
	cansmb_send_data(cached_ack,cached_ack_size);pfc_last_activity=sys_millis();
	pending_request_size=0;
	if(ack_cmd==0)gateway_close();
	else if(ack_cmd==8 || ack_cmd==0x22 || ack_cmd==0x24)
	{pfc_state=PFC_GW_WAIT_RESET;pfc_state_started=sys_millis();}
	else pfc_state=PFC_GW_IDLE;
	pfc_parser_reset();
}

static void pfc_receive_byte(uchar byte)
{
	ushort payload;
	if(pfc_rx_size==0 && byte!=MODBUS_PFC_ADDR)return;
	if(pfc_rx_size>=sizeof(pfc_rx)){pfc_parser_reset();return;}
	pfc_rx[pfc_rx_size++]=byte;
	if(pfc_rx_size==2 && pfc_rx[1]!=FNO_IAP){pfc_parser_reset();return;}
	if(pfc_rx_size==12)
	{
		payload=(ushort)pfc_rx[10]|((ushort)pfc_rx[11]<<8);
		if(payload>MODBUS_MAX_REG_PAYLOAD_SIZE){pfc_parser_reset();return;}
		pfc_rx_expected=(ushort)(payload+IAP_PACKET_OVERHEAD);
	}
	if(pfc_rx_expected && pfc_rx_size==pfc_rx_expected)forward_pfc_ack();
}

static void on_iap_cmd(modbus_iap_t *pkt)
{
	uchar request[sizeof(modbus_iap_t)];ushort request_size,command;
	if(pkt->addr!=MODBUS_PFC_ADDR)
	{
		iap_pkt_decode((iap_pkt_t*)pkt->cmd);
		if(pkt->cmd[0]!=0xff || pkt->cmd[1]!=0xff)
			modbuss_send_iap(pkt->cmd,(const void*)pkt->data,pkt->size);
		return;
	}
	request_size=build_packet(pkt,request);if(request_size==0)return;
	command=(ushort)pkt->cmd[0]|((ushort)pkt->cmd[1]<<8);
	pfc_forward_mode=true;pfc_last_activity=sys_millis();
	if(packet_equal(request,request_size,cached_request,cached_request_size))
	{cansmb_send_data(cached_ack,cached_ack_size);return;}
	if(pfc_state==PFC_GW_WAIT_ACK)return;
	memmove(pending_request,request,request_size);pending_request_size=request_size;
	pending_command=command;pfc_parser_reset();
	uart0_send_data(pending_request,pending_request_size);
	pfc_state=PFC_GW_WAIT_ACK;pfc_state_started=sys_millis();
}

void modbus_iap_init()
{
	pfc_forward_mode=false;pfc_state=PFC_GW_IDLE;pfc_state_started=0;pfc_last_activity=0;
	pending_request_size=0;cached_request_size=0;cached_ack_size=0;pfc_parser_reset();
	modbuss_init(MODBUS_LOCAL_ADDR);modbuss_set_timeout(1000);
	modbuss_set_iap_callback(on_iap_cmd);modbuss_set_send_callback(cansmb_send_data);
	cansmb_init(iap_runtime_can_id(),125000);uart0_init(115200);
}

bool iap_pfc_forward_active(void){return pfc_forward_mode;}

void modbus_iap_task()
{
	uchar data;ulong now=sys_millis();
	modbuss_task();while(uart0_rx_poll(&data))pfc_receive_byte(data);uart0_tx_poll();
	if(pfc_state==PFC_GW_WAIT_ACK && (ulong)(now-pfc_state_started)>=PFC_RESPONSE_TIMEOUT_MS)
	{
		if(pending_command==0x25)gateway_close();
		else {pfc_state=PFC_GW_IDLE;pending_request_size=0;pfc_parser_reset();}
	}
	else if(pfc_state==PFC_GW_WAIT_RESET && (ulong)(now-pfc_state_started)>=PFC_RESET_TIMEOUT_MS)
	{pfc_state=PFC_GW_IDLE;}
	if(pfc_forward_mode && pfc_state==PFC_GW_IDLE &&
	   (ulong)(now-pfc_last_activity)>=PFC_GATEWAY_IDLE_MS)gateway_close();
}
