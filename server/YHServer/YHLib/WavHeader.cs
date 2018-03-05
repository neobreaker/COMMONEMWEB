using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Runtime.InteropServices;

namespace YHServer
{
    //riff
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ChunkRIFF
    {
        public UInt32 ChunkID;
        public UInt32 ChunkSize;
        public UInt32 Format;	   		
    } ;

    //fmt
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ChunkFMT
    {
        public UInt32 ChunkID;
        public UInt32 ChunkSize;
        public UInt16 AudioFormat;
        public UInt16 NumOfChannels;
        public UInt32 SampleRate;
        public UInt32 ByteRate;
        public UInt16 BlockAlign;
        public Int16 BitsPerSample;	
    //	u16 ByteExtraData;		
    //	u16 ExtraData;			
    };	

    //fact 
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ChunkFACT
    {
        public UInt32 ChunkID;
        public UInt32 ChunkSize;
        public UInt32 NumOfSamples;	
    };

    //data 
    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct ChunkDATA
    {
        public UInt32 ChunkID;
        public UInt32 ChunkSize;		
    };

    [StructLayout(LayoutKind.Sequential, Pack = 1)]
    public struct WavHeaderStruct
    {
         public ChunkRIFF riff;
         public ChunkFMT fmt;
         public ChunkDATA data;
    };

    public class WavHeader
    {
        public byte[] GetWavHeader()
        {
            WavHeaderStruct wavhead = new WavHeaderStruct();

            UInt32 data_sz = 1024*1024;
            UInt32 riff_sz = data_sz + 36;
            
            wavhead.riff.ChunkID=0X46464952;
            wavhead.riff.ChunkSize=riff_sz;
            wavhead.riff.Format=0X45564157;
            wavhead.fmt.ChunkID=0X20746D66;
            wavhead.fmt.ChunkSize=16; 
            wavhead.fmt.AudioFormat=0X01;
            wavhead.fmt.NumOfChannels=1;
            wavhead.fmt.SampleRate=8000;
            wavhead.fmt.ByteRate=wavhead.fmt.SampleRate*2;
            wavhead.fmt.BlockAlign=2;	
            wavhead.fmt.BitsPerSample=16;
            wavhead.data.ChunkID=0X61746164;
            wavhead.data.ChunkSize=data_sz;

            byte[] ret = Struct2Byte(wavhead);

            return ret;
        }

        private byte[] Struct2Byte(WavHeaderStruct s)
        {
            int structSize = Marshal.SizeOf(typeof(WavHeaderStruct));
            byte[] buffer = new byte[structSize];
            //分配结构体大小的内存空间
            IntPtr structPtr = Marshal.AllocHGlobal(structSize);
            //将结构体拷到分配好的内存空间
            Marshal.StructureToPtr(s, structPtr, false);
            //从内存空间拷到byte数组
            Marshal.Copy(structPtr, buffer, 0, structSize);
            //释放内存空间
            Marshal.FreeHGlobal(structPtr);
            return buffer;
        }
    }

}
