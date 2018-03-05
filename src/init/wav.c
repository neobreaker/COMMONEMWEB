#include "wav.h"


void wav_header_init(WaveHeaderStruct* wavhead, u32 riff_sz, u32 data_sz)	   
{
	wavhead->riff.ChunkID=0X46464952;	//"RIFF"
	wavhead->riff.ChunkSize=riff_sz;			//还未确定,最后需要计算	整个文件的大小-8;
	wavhead->riff.Format=0X45564157; 	//"WAVE"
	wavhead->fmt.ChunkID=0X20746D66; 	//"fmt "
	wavhead->fmt.ChunkSize=16; 			//大小为16个字节
	wavhead->fmt.AudioFormat=0X01; 		//0X01,表示PCM;0X01,表示IMA ADPCM
 	wavhead->fmt.NumOfChannels=1;		//单声道
 	wavhead->fmt.SampleRate=8000;		//8Khz采样率 采样速率
 	wavhead->fmt.ByteRate=wavhead->fmt.SampleRate*2;//16位,即2个字节
 	wavhead->fmt.BlockAlign=2;			//块大小,2个字节为一个块
 	wavhead->fmt.BitsPerSample=16;		//16位PCM
   	wavhead->data.ChunkID=0X61746164;	//"data"
 	wavhead->data.ChunkSize=data_sz;			//数据大小,还需要计算  数据大小

}

