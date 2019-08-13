#include <string.h>
#include <stdint.h>
#include "cvsd.h"
#include "math.h"

T_CVSD_MAIN_STRUCT EncodeParams;

#define DELTA_MIN           (int32_t)(0.0001 * 32768)//(0.0002 * 32768)
#define DELTA_MAX           (int32_t)(0.0078 * 32768)//(0.0078 * 32768)
#define SYLLABIC_CONST      (int32_t)(0.9845 * 32768)//(0.9845 * 32768)
#define PRM_INTEG_CONST     (int32_t)(0.9394 * 32768)//(0.9394 * 32768)
#define INTEG_B1            (int32_t)(1.2708 * 32768)//(1.2708 * 32768)
#define INTEG_B2            (int32_t)(0.3202 * 32768)//(0.3202 * 32768)
#define INTEG_G2D           (int32_t)(1.5092 * 32768)//(1.5092 * 32768)

void cvsd_encode(register int16_t *voice_frame, register uint8_t *cvsd_out_pack, register size_t len)
{
	memset(&EncodeParams, 0, sizeof(EncodeParams));
	register int16_t current_bit;
	register int32_t tmp1, tmp2;
	register size_t i;

	
	for (i = 0; i<len; i++)
	{
		EncodeParams.In_current = voice_frame[i];

		if (EncodeParams.In_current >(EncodeParams.product))
			current_bit = 1;
		else
			current_bit = -1;

		EncodeParams.bit_accum = current_bit + EncodeParams.prev1 + EncodeParams.prev2;

		tmp1 = (SYLLABIC_CONST * EncodeParams.step) >> 15;

		if (abs(EncodeParams.bit_accum) == 3)
			EncodeParams.step = tmp1 + DELTA_MAX;
		else
			EncodeParams.step = tmp1 + DELTA_MIN;
		// Primary reconstruction integration
		tmp1 = (PRM_INTEG_CONST * EncodeParams.product) >> 15;
		tmp2 = EncodeParams.step*current_bit;

		EncodeParams.product = tmp1 + tmp2;

		// Shift 
		EncodeParams.prev2 = EncodeParams.prev1;
		EncodeParams.prev1 = current_bit;

		EncodeParams.Out_current = (current_bit + 1) >> 1; // -1 --> 0; 1 --> 1

		cvsd_out_pack[i >> 3] <<= 1;
		cvsd_out_pack[i >> 3] |= EncodeParams.Out_current; // write bit to the beginning of the file
	}
}

void cvsd_decode(int16_t *voice_frame, uint8_t *cvsd_out_pack, size_t len)
{
	int32_t tmp1, tmp2, tmp3;
	size_t i;

	for (i = 0; i<len; i++)
	{
		EncodeParams.In_current = (cvsd_out_pack[i / 8] >> (7 - i % 8)) & 1; //extract current bit from byte

		EncodeParams.In_current = 2 * EncodeParams.In_current - 1;   // 0 --> -1; 1 --> 1

		EncodeParams.bit_accum = EncodeParams.In_current + EncodeParams.prev1 + EncodeParams.prev2;

		tmp1 = (SYLLABIC_CONST * EncodeParams.dec_step) >> 15;

		if (abs(EncodeParams.bit_accum) == 3)
			EncodeParams.dec_step = tmp1 + DELTA_MAX;
		else
			EncodeParams.dec_step = tmp1 + DELTA_MIN;

		//	Primary reconstruction integration
		tmp1 = (INTEG_B1 * EncodeParams.dec_prev1) >> 15;
		tmp2 = (INTEG_B2 * EncodeParams.dec_prev2) >> 15;

		tmp3 = (INTEG_G2D * EncodeParams.dec_step) >> 15;
		tmp3 = tmp3 * EncodeParams.In_current;

		tmp1 = tmp1 - tmp2 + tmp3;

		//	Saturation process
		if (tmp1 >= 32767)        // up overflow
			EncodeParams.Out_current = 32767;
		else if (tmp1 <= -32768)  // down overflow
			EncodeParams.Out_current = -32768;
		else                      // no overflow
			EncodeParams.Out_current = tmp1;

		//	Shift 
		EncodeParams.prev2 = EncodeParams.prev1;
		EncodeParams.prev1 = EncodeParams.In_current;

		EncodeParams.dec_prev2 = EncodeParams.dec_prev1;
		EncodeParams.dec_prev1 = EncodeParams.Out_current;

		voice_frame[i] = EncodeParams.Out_current;
	}
}
