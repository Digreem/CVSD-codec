# CVSD-codec
Continuously variable slope delta modulation (CVSD or CVSDM) is a voice coding method.   
It is a delta modulation with variable step size (i.e., special case of adaptive delta modulation)

CVSD encodes at 1 bit per sample, so that audio sampled at 16 kHz is encoded at 16 kbit/s.

Program input is a .wav file containing a speech recording.   
Output is a degraded after decoding process file. So we can estimate degradation level aurally or using   
special tests (MBSD, PESQ).
