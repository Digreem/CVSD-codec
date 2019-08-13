typedef struct
{
	int16_t prev1;
	int16_t prev2;
	int32_t step;
	int32_t product;
	int16_t In_current;
	int16_t Out_current;
	int32_t bit_accum;
	int16_t dec_step;
	int16_t dec_prev1;
	int16_t dec_prev2;
} T_CVSD_MAIN_STRUCT;

//void cvsd_encode(int16_t *voice_frame, uint8_t *cvsd_out_pack, size_t len);
//void cvsd_decode(int16_t *voice_frame, uint8_t *cvsd_out_pack, size_t len);
void cvsd_encode(register int16_t *voice_frame, register uint8_t *cvsd_out_pack, register size_t len);
void cvsd_decode(int16_t *voice_frame, uint8_t *cvsd_out_pack, size_t len);