#include "variables_define_app.h"

PFC_HRPWM_TypeDef mpwm, pwm0, pwm1,  pwm2, pwm3, pwm4, pwm5;
SPLL_1PH_NOTCH spll_1ph_notch;

int16_t  acl_samp [BUF_LEN] 			__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t  acn_samp [BUF_LEN] 			__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t  i_surge_samp [BUF_LEN] 	__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t  i_cr_samp [BUF_LEN]			__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t	 vbus_samp [BUF_LEN]			__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t	 i_ocp_samp [BUF_LEN]			__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t	 r_ntc_samp [BUF_LEN]			__attribute__((aligned(16), section("SECTION_RAMB")));
int16_t	 i_samp [BUF_LEN]					__attribute__((aligned(16), section("SECTION_RAMB")));

int16_t spll_iir_indata					 __attribute__((aligned(4), section("SECTION_RAMB")));
int16_t vbus_notch_in						 __attribute__((aligned(4), section("SECTION_RAMB")));
int16_t vbus_notch_out					 __attribute__((aligned(4), section("SECTION_RAMB")));

uint32_t pfc_vloop_kp1;
uint32_t pfc_vloop_ki1;

uint32_t fre_cnt;
uint32_t irq_cnt;
uint32_t start_cnt;
uint32_t led_cnt;
volatile pfcTypeDef pfc;
volatile PFC_TXDATA_TypeDef txdata_buf;
volatile PFC_RXDATA_TypeDef rxdata_buf;

bool pfc_current_integral_keep_en;
bool pfc_current_integral_add_en;


int16_t cosdata[LEN_COSSIN_BUF],sindata[LEN_COSSIN_BUF];
volatile uint8_t TXBuffer[TXBUFFER_LEN], RXBuffer[RXBUFFER_LEN];
//ADC filter
//#define CUR_FLT_LEN	10
//#define VOL_FLT_LEN	10
//uint8_t TX_BUF[5];// = {0,1,2,3,4,5,6,7,8,9};
//uint8_t RX_BUF[10];
