#include <iostream>
#include <cstdio>
#include <stdint.h>
#include <cmath>
#include "string.h"
#include "cvsd.h"

struct chunk_t
{
	char ID[4]; //"data" = 0x61746164
	unsigned long size;  //Chunk data bytes
};
//Wav Header
struct wav_header_t
{
	char chunkID[4]; //"RIFF" = 0x46464952
	unsigned long chunkSize; //28 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes] + sum(sizeof(chunk.id) + sizeof(chunk.size) + chunk.size)
	char format[4]; //"WAVE" = 0x45564157
	char subchunk1ID[4]; //"fmt " = 0x20746D66
	unsigned long subchunk1Size; //16 [+ sizeof(wExtraFormatBytes) + wExtraFormatBytes]
	unsigned short audioFormat;
	unsigned short numChannels;
	unsigned long sampleRate;
	unsigned long byteRate;
	unsigned short blockAlign;
	unsigned short bitsPerSample;
	//[WORD wExtraFormatBytes;]
	//[Extra format bytes]
	chunk_t  subchunk2;
};

struct wav_param
{
	wav_header_t header;
	int samples_count;
};

//Chunks
int16_t *WavRead(const char* fileName, wav_param *param)
{
	FILE *fin = fopen(fileName, "rb");

	//Read WAV header
	wav_header_t header;
	chunk_t chunk;
	fread(&header, sizeof(header)-sizeof(chunk), 1, fin);

	//Print WAV header
	printf("WAV File Header read:\n");
	printf("File Type: %s\n", header.chunkID);
	printf("File Size: %ld\n", header.chunkSize);
	printf("WAV Marker: %s\n", header.format);
	printf("Format Name: %s\n", header.subchunk1ID);
	printf("Format Length: %ld\n", header.subchunk1Size);
	printf("Format Type: %hd\n", header.audioFormat);
	printf("Number of Channels: %hd\n", header.numChannels);
	printf("Sample Rate: %ld\n", header.sampleRate);
	printf("Sample Rate * Bits/Sample * Channels / 8: %ld\n", header.byteRate);
	printf("Bits per Sample * Channels / 8.1: %hd\n", header.blockAlign);
	printf("Bits per Sample: %hd\n", header.bitsPerSample);

	//skip wExtraFormatBytes & extra format bytes
	//fseek(f, header.chunkSize - 16, SEEK_CUR);

	//Reading file
	
	printf("id\t" "size\n");
	//go to data chunk
	while (true)
	{
		fread(&chunk, sizeof(chunk), 1, fin);
		printf("%c%c%c%c\t" "%li\n", chunk.ID[0], chunk.ID[1], chunk.ID[2], chunk.ID[3], chunk.size);
		if (*(unsigned int *)&chunk.ID == 0x61746164)
			break;
		//skip chunk data bytes
		fseek(fin, chunk.size, SEEK_CUR);
	}
	header.subchunk2 = chunk;
	//Number of samples
	int sample_size = header.bitsPerSample / 8;
	int samples_count = chunk.size * 8 / header.bitsPerSample;
	printf("Samples count = %i\n", samples_count);

	param->header = header;
	param->samples_count = samples_count;


	short int *value = new short int[samples_count];
	memset(value, 0, sizeof(short int)* samples_count);

	//Reading data
	for (int i = 0; i < samples_count; i++)
	{
		fread(&value[i], sample_size, 1, fin);
	}
	fclose(fin);
	return value;
}

void WavWrite(const char* fileToSave, int16_t* value, wav_param *param)
{
	FILE *fout = fopen(fileToSave, "wb");

	fwrite(&(param->header), sizeof(param->header), 1, fout);
	
	for (int i = 0; i < param->samples_count; i++)
	{
		fwrite(&value[i], sizeof(int16_t), 1, fout);
	}
	fclose(fout);
}

int main()
{
	int16_t *OrigData;
	uint8_t *EncodeData;
	int16_t *DecodeData;
	wav_param param;
	OrigData = WavRead("voice_16k.wav", &param);
	int EncodeSize = param.samples_count / 8 + 1; //number of bytes in encoded data
	EncodeData = new uint8_t[EncodeSize];
	DecodeData = new int16_t[param.samples_count];
	cvsd_encode(OrigData, EncodeData, param.samples_count);
	//can write an encoded file or transmit over a transmission channel
	cvsd_decode(DecodeData, EncodeData, param.samples_count);
	WavWrite("degraded.wav", DecodeData, &param);

	return 0;
}