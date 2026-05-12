#include "variables_define_app.h"
#include "main.h"

LLC_HRPWM_TypeDef mpwm, pwm0, pwm1,  pwm2, pwm3, pwm4, pwm5;
llcTypeDef llc;

int16_t temp_samp				[BUF_LEN]	__attribute__((aligned(8), section("SECTION_RAMB")));
int16_t s_trim_samp			[BUF_LEN]	__attribute__((aligned(8), section("SECTION_RAMB")));
int16_t vout_samp				[BUF_LEN]	__attribute__((aligned(8), section("SECTION_RAMB")));
int16_t vfb_samp				[BUF_LEN]	__attribute__((aligned(8), section("SECTION_RAMB")));
int16_t loadshare_samp	[BUF_LEN] __attribute__((aligned(8), section("SECTION_RAMB")));
int16_t iout_samp				[BUF_LEN]	__attribute__((aligned(8), section("SECTION_RAMB")));
int16_t can_addr1_samp	[BUF_LEN] __attribute__((aligned(8), section("SECTION_RAMB")));
int16_t can_addr2_samp	[BUF_LEN] __attribute__((aligned(8), section("SECTION_RAMB")));
int16_t can_addr3_samp	[BUF_LEN] __attribute__((aligned(8), section("SECTION_RAMB")));
int16_t sr_temp					[BUF_LEN] __attribute__((aligned(8), section("SECTION_RAMB")));

