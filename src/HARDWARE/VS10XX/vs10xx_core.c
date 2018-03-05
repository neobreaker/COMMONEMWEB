#include "vs10xx_core.h"
#include "delay.h"


//VS10XXĬ�����ò���
_vs10xx_obj vsset=
{
	220,	//����:220
	6,		//�������� 60Hz
	15,		//�������� 15dB	
	10,		//�������� 10Khz	
	15,		//�������� 10.5dB
	0,		//�ռ�Ч��	
	1,		//��������Ĭ�ϴ�.
};

////////////////////////////////////////////////////////////////////////////////
//��λVS10XX
void VS_Soft_Reset(vs10xx_cfg_t *cfg)
{
    u8 retry=0;
    cfg->VS_WAIT_DQ();                          //�ȴ������λ����
    cfg->VS_SPI_ReadWriteByte(0Xff);            //��������

    retry = 0;
    while(VS_RD_Reg(cfg, SPI_MODE)!=0x0800)         // �����λ,��ģʽ
    {
        VS_WR_Cmd(cfg, SPI_MODE,0x0804);        // �����λ,��ģʽ
        delay_ms(2);                            //�ȴ�����1.35ms
        if(retry++>100)break;
    }

    cfg->VS_WAIT_DQ();                          //�ȴ������λ����
    retry=0;
    while(VS_RD_Reg(cfg, SPI_CLOCKF)!=0X9800)   //����VS10XX��ʱ��,3��Ƶ ,1.5xADD
    {
        VS_WR_Cmd(cfg, SPI_CLOCKF,0X9800);      //����VS10XX��ʱ��,3��Ƶ ,1.5xADD
        if(retry++>100)break;
    }
    delay_ms(20);
}

//Ӳ��λMP3
//����1:��λʧ��;0:��λ�ɹ�
u8 VS_HD_Reset(vs10xx_cfg_t *cfg)
{
    u8 retry = 0;
    cfg->VS_RST(0);
    delay_ms(20);
    cfg->VS_XDCS(1);        //ȡ�����ݴ���
    cfg->VS_XCS(1);         //ȡ�����ݴ���
    cfg->VS_RST(1);

    while(cfg->VS_DQ()==0 &&retry<200)//�ȴ�DREQΪ��
    {
        retry++;
        delay_us(50);
    };

    delay_ms(20);
    if(retry>=200)
        return 1;
    else
        return 0;
}

//���Ҳ���
void VS_Sine_Test(vs10xx_cfg_t *cfg)
{
    VS_HD_Reset(cfg);
    VS_WR_Cmd(cfg, 0x0b,0X2020);                    //��������
    VS_WR_Cmd(cfg, SPI_MODE,0x0820);                //����VS10XX�Ĳ���ģʽ

    while(cfg->VS_DQ() == 0);                               //�ȴ�DREQΪ��

    //��VS10XX�������Ҳ������0x53 0xef 0x6e n 0x00 0x00 0x00 0x00
    //����n = 0x24, �趨VS10XX�����������Ҳ���Ƶ��ֵ��������㷽����VS10XX��datasheet


    cfg->VS_SPI_SpeedLow();         //����
    cfg->VS_XDCS(0);                //ѡ�����ݴ���
    cfg->VS_SPI_ReadWriteByte(0x53);
    cfg->VS_SPI_ReadWriteByte(0xef);
    cfg->VS_SPI_ReadWriteByte(0x6e);
    cfg->VS_SPI_ReadWriteByte(0x24);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    delay_ms(100);
    cfg->VS_XDCS(1);                //�˳����Ҳ���

    cfg->VS_XDCS(0);                //ѡ�����ݴ���
    cfg->VS_SPI_ReadWriteByte(0x45);
    cfg->VS_SPI_ReadWriteByte(0x78);
    cfg->VS_SPI_ReadWriteByte(0x69);
    cfg->VS_SPI_ReadWriteByte(0x74);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    delay_ms(100);
    cfg->VS_XDCS(1);

    //�ٴν������Ҳ��Բ�����nֵΪ0x44���������Ҳ���Ƶ������Ϊ�����ֵ
    cfg->VS_XDCS(0);                //ѡ�����ݴ���
    cfg->VS_SPI_ReadWriteByte(0x53);
    cfg->VS_SPI_ReadWriteByte(0xef);
    cfg->VS_SPI_ReadWriteByte(0x6e);
    cfg->VS_SPI_ReadWriteByte(0x44);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    delay_ms(100);
    cfg->VS_XDCS(1);                        //�˳����Ҳ���

    cfg->VS_XDCS(0);                        //ѡ�����ݴ���
    cfg->VS_SPI_ReadWriteByte(0x45);
    cfg->VS_SPI_ReadWriteByte(0x78);
    cfg->VS_SPI_ReadWriteByte(0x69);
    cfg->VS_SPI_ReadWriteByte(0x74);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    delay_ms(100);
    cfg->VS_XDCS(1);
}

//ram ����
//����ֵ:RAM���Խ��
// VS1003����õ���ֵΪ0x807F����������;VS1053Ϊ0X83FF.
u16 VS_Ram_Test(vs10xx_cfg_t *cfg)
{
    VS_HD_Reset(cfg);
    VS_WR_Cmd(cfg, SPI_MODE,0x0820);        // ����VS10XX�Ĳ���ģʽ
    while (cfg->VS_DQ()==0);                // �ȴ�DREQΪ��
    cfg->VS_SPI_SpeedLow();                     //����
    cfg->VS_XDCS(0);                             // xDCS = 1��ѡ��VS10XX�����ݽӿ�
    cfg->VS_SPI_ReadWriteByte(0x4d);
    cfg->VS_SPI_ReadWriteByte(0xea);
    cfg->VS_SPI_ReadWriteByte(0x6d);
    cfg->VS_SPI_ReadWriteByte(0x54);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    cfg->VS_SPI_ReadWriteByte(0x00);
    delay_ms(150);
    cfg->VS_XDCS(1);
    return VS_RD_Reg(cfg, SPI_HDAT0);       // VS1003����õ���ֵΪ0x807F����������;VS1053Ϊ0X83FF.;
}


//��VS10XXд����
//address:�����ַ
//data:��������
void VS_WR_Cmd(vs10xx_cfg_t *cfg, u8 address,u16 data)
{
    while(cfg->VS_DQ()==0);                        //�ȴ�����
    cfg->VS_SPI_SpeedLow();                      //����
    cfg->VS_XDCS(1);
    cfg->VS_XCS(0);
    cfg->VS_SPI_ReadWriteByte(VS_WRITE_COMMAND); //����VS10XX��д����
    cfg->VS_SPI_ReadWriteByte(address);          //��ַ
    cfg->VS_SPI_ReadWriteByte(data>>8);          //���͸߰�λ
    cfg->VS_SPI_ReadWriteByte(data);             //�ڰ�λ
    cfg->VS_XCS(1);
    cfg->VS_SPI_SpeedHigh();                     //����
}

//��VS10XXд����
//data:Ҫд�������
void VS_WR_Data(vs10xx_cfg_t *cfg, u8 data)
{
    cfg->VS_SPI_SpeedHigh();//����,��VS1003B,���ֵ���ܳ���36.864/4Mhz����������Ϊ9M
    cfg->VS_XDCS(0);
    cfg->VS_SPI_ReadWriteByte(data);
    cfg->VS_XDCS(1);
}

//��VS10XX�ļĴ���
//address���Ĵ�����ַ
//����ֵ��������ֵ
//ע�ⲻҪ�ñ��ٶ�ȡ,�����
u16 VS_RD_Reg(vs10xx_cfg_t *cfg, u8 address)
{
    u16 temp=0;
    while(cfg->VS_DQ()==0);            //�ǵȴ�����״̬
    cfg->VS_SPI_SpeedLow();          //����
    cfg->VS_XDCS(1);
    cfg->VS_XCS(0);
    cfg->VS_SPI_ReadWriteByte(VS_READ_COMMAND);  //����VS10XX�Ķ�����
    cfg->VS_SPI_ReadWriteByte(address);          //��ַ
    temp=cfg->VS_SPI_ReadWriteByte(0xff);        //��ȡ���ֽ�
    temp=temp<<8;
    temp+=cfg->VS_SPI_ReadWriteByte(0xff);       //��ȡ���ֽ�
    cfg->VS_XCS(1);
    cfg->VS_SPI_SpeedHigh();         //����
    return temp;
}

//��ȡVS10xx��RAM
//addr��RAM��ַ
//����ֵ��������ֵ
u16 VS_WRAM_Read(vs10xx_cfg_t *cfg, u16 addr)
{
    u16 res;
    VS_WR_Cmd(cfg, SPI_WRAMADDR, addr);
    res=VS_RD_Reg(cfg, SPI_WRAM);
    return res;
}

//дVS10xx��RAM
//addr��RAM��ַ
//val:Ҫд���ֵ
void VS_WRAM_Write(vs10xx_cfg_t *cfg, u16 addr,u16 val)
{
    VS_WR_Cmd(cfg, SPI_WRAMADDR, addr);   //дRAM��ַ
    while(cfg->VS_DQ() == 0);                //�ȴ�����
    VS_WR_Cmd(cfg, SPI_WRAM, val);        //дRAMֵ
}

//���ò����ٶȣ���VS1053��Ч��
//t:0,1,�����ٶ�;2,2���ٶ�;3,3���ٶ�;4,4����;�Դ�����
void VS_Set_Speed(vs10xx_cfg_t *cfg, u8 t)
{
    VS_WRAM_Write(cfg, 0X1E04,t);        //д�벥���ٶ�
}

//FOR WAV HEAD0 :0X7761 HEAD1:0X7665
//FOR MIDI HEAD0 :other info HEAD1:0X4D54
//FOR WMA HEAD0 :data speed HEAD1:0X574D
//FOR MP3 HEAD0 :data speed HEAD1:ID
//������Ԥ��ֵ,�ײ�III
const u16 bitrate[2][16]=
{
    {0,8,16,24,32,40,48,56,64,80,96,112,128,144,160,0},
    {0,32,40,48,56,64,80,96,112,128,160,192,224,256,320,0}
};

//����Kbps�Ĵ�С
//����ֵ���õ�������
u16 VS_Get_HeadInfo(vs10xx_cfg_t *cfg)
{
    unsigned int HEAD0;
    unsigned int HEAD1;
    HEAD0=VS_RD_Reg(cfg, SPI_HDAT0);
    HEAD1=VS_RD_Reg(cfg, SPI_HDAT1);
    
    switch(HEAD1)
    {
        case 0x7665://WAV��ʽ
        case 0X4D54://MIDI��ʽ
        case 0X4154://AAC_ADTS
        case 0X4144://AAC_ADIF
        case 0X4D34://AAC_MP4/M4A
        case 0X4F67://OGG
        case 0X574D://WMA��ʽ
        case 0X664C://FLAC��ʽ
        {

            HEAD1=HEAD0*2/25;//�൱��*8/100
            if((HEAD1%10)>5)
				return HEAD1/10+1;//��С�����һλ��������
            else 
				return HEAD1/10;
        }
        default://MP3��ʽ,�����˽ײ�III�ı�
        {
            HEAD1>>=3;
            HEAD1=HEAD1&0x03;
            if(HEAD1==3)
				HEAD1=1;
            else
				HEAD1=0;
			
            return bitrate[HEAD1][HEAD0>>12];
        }
    }
}

//�õ�ƽ���ֽ���
//����ֵ��ƽ���ֽ����ٶ�
u32 VS_Get_ByteRate(vs10xx_cfg_t *cfg)
{
    return VS_WRAM_Read(cfg, 0X1E05);//ƽ��λ��
}

//�õ���Ҫ��������
//����ֵ:��Ҫ��������
u16 VS_Get_EndFillByte(vs10xx_cfg_t *cfg)
{
    return VS_WRAM_Read(cfg, 0X1E06);//����ֽ�
}

//����һ����Ƶ����
//�̶�Ϊ32�ֽ�
//����ֵ:0,���ͳɹ�
//       1,VS10xx��ȱ����,��������δ�ɹ�����
u8 VS_Send_MusicData(vs10xx_cfg_t *cfg, u8* buf)
{
    u8 n;

	cfg->VS_WAIT_DQ();
	
    if(cfg->VS_DQ() != 0)  //�����ݸ�VS10XX
    {
        cfg->VS_XDCS(0);
        for(n = 0; n < SEND_NUM_PER_FRAME; n++)
        {
            cfg->VS_SPI_ReadWriteByte(buf[n]);
        }
        cfg->VS_XDCS(1);
    }
    else 
		return 1;
	
    return 0;//�ɹ�������
}

u8 VS_Send_MusicData2(vs10xx_cfg_t *cfg, u8* buf, int len)	//len must less equal 32
{
    u8 n;
    if(cfg->VS_DQ() != 0)  //�����ݸ�VS10XX
    {
        cfg->VS_XDCS(0);
        for(n = 0; n < len; n++)
        {
            cfg->VS_SPI_ReadWriteByte(buf[n]);
        }
        cfg->VS_XDCS(1);
    }
    else 
		return 1;
	
    return 0;//�ɹ�������
}

//�и�
//ͨ���˺����и裬��������л���������
void VS_Restart_Play(vs10xx_cfg_t *cfg)
{
    u16 temp;
    u16 i;
    u8 n;
    u8 vsbuf[32];
	
    for(n=0; n<32; n++)
		vsbuf[n]=0; //����
		
    temp=VS_RD_Reg(cfg, SPI_MODE);   //��ȡSPI_MODE������
    temp|=1<<3;                 //����SM_CANCELλ
    temp|=1<<2;                 //����SM_LAYER12λ,������MP1,MP2
    
    VS_WR_Cmd(cfg, SPI_MODE, temp);   //����ȡ����ǰ����ָ��
    
    for(i=0; i<2048; )           //����2048��0,�ڼ��ȡSM_CANCELλ.���Ϊ0,���ʾ�Ѿ�ȡ���˵�ǰ����
    {
        if(VS_Send_MusicData(cfg, vsbuf)==0)//ÿ����32���ֽں���һ��
        {
            i+=32;                      		//������32���ֽ�
            temp=VS_RD_Reg(cfg, SPI_MODE);   	//��ȡSPI_MODE������
            if((temp&(1<<3))==0)
				break;  						//�ɹ�ȡ����
        }
    }
    if(i<2048)//SM_CANCEL����
    {
        temp=VS_Get_EndFillByte(cfg)&0xff;//��ȡ����ֽ�
        for(n=0; n<32; n++)
			vsbuf[n]=temp; //����ֽڷ�������
			
        for(i=0; i<2052;)
        {
            if(VS_Send_MusicData(cfg, vsbuf)==0)i+=32;//���
        }
    }
    else 
		VS_Soft_Reset(cfg);       //SM_CANCEL���ɹ�,�����,��Ҫ��λ
		
    temp=VS_RD_Reg(cfg, SPI_HDAT0);
    temp+=VS_RD_Reg(cfg, SPI_HDAT1);
    if(temp)                       //��λ,����û�гɹ�ȡ��,��ɱ���,Ӳ��λ
    {
        VS_HD_Reset(cfg);          //Ӳ��λ
        VS_Soft_Reset(cfg);        //��λ
    }
}

//�������ʱ��
void VS_Reset_DecodeTime(vs10xx_cfg_t *cfg)
{
    VS_WR_Cmd(cfg, SPI_DECODE_TIME,0x0000);
    VS_WR_Cmd(cfg, SPI_DECODE_TIME,0x0000);//��������
}

//�õ�mp3�Ĳ���ʱ��n sec
//����ֵ������ʱ��
u16 VS_Get_DecodeTime(vs10xx_cfg_t *cfg)
{
    u16 dt=0;
    dt=VS_RD_Reg(cfg, SPI_DECODE_TIME);
    return dt;
}

//vs10xxװ��patch.
//patch��patch�׵�ַ
//len��patch����
void VS_Load_Patch(vs10xx_cfg_t *cfg, u16 *patch,u16 len)
{
    u16 i;
    u16 addr, n, val;
    for(i=0; i<len;)
    {
        addr = patch[i++];
        n    = patch[i++];
        if(n & 0x8000U) //RLE run, replicate n samples
        {
            n  &= 0x7FFF;
            val = patch[i++];
            while(n--)VS_WR_Cmd(cfg, addr, val);
        }
        else  //copy run, copy n sample
        {
            while(n--)
            {
                val = patch[i++];
                VS_WR_Cmd(cfg, addr, val);
            }
        }
    }
}

//�趨VS10XX���ŵ������͸ߵ���
//volx:������С(0~254)
void VS_Set_Vol(vs10xx_cfg_t *cfg, u8 volx)
{
    u16 volt=0;             //�ݴ�����ֵ
    volt=254-volx;          //ȡ��һ��,�õ����ֵ,��ʾ���ı�ʾ
    volt<<=8;
    volt+=254-volx;         //�õ��������ú��С
    VS_WR_Cmd(cfg, SPI_VOL,volt);//������
}

//�趨�ߵ�������
//bfreq:��Ƶ����Ƶ��    2~15(��λ:10Hz)
//bass:��Ƶ����         0~15(��λ:1dB)
//tfreq:��Ƶ����Ƶ��    1~15(��λ:Khz)
//treble:��Ƶ����       0~15(��λ:1.5dB,С��9��ʱ��Ϊ����)
void VS_Set_Bass(vs10xx_cfg_t *cfg, u8 bfreq,u8 bass,u8 tfreq,u8 treble)
{
    u16 bass_set=0; //�ݴ������Ĵ���ֵ
    signed char temp=0;
    if(treble==0)temp=0;            //�任
    else if(treble>8)temp=treble-8;
    else temp=treble-9;
    bass_set=temp&0X0F;             //�����趨
    bass_set<<=4;
    bass_set+=tfreq&0xf;            //��������Ƶ��
    bass_set<<=4;
    bass_set+=bass&0xf;             //�����趨
    bass_set<<=4;
    bass_set+=bfreq&0xf;            //��������
    VS_WR_Cmd(cfg, SPI_BASS, bass_set);   //BASS
}

//�趨��Ч
//eft:0,�ر�;1,��С;2,�е�;3,���.
void VS_Set_Effect(vs10xx_cfg_t *cfg, u8 eft)
{
    u16 temp;
    temp=VS_RD_Reg(cfg, SPI_MODE);   //��ȡSPI_MODE������
    if(eft&0X01)temp|=1<<4;     //�趨LO
    else temp&=~(1<<5);         //ȡ��LO
    if(eft&0X02)temp|=1<<7;     //�趨HO
    else temp&=~(1<<7);         //ȡ��HO
    VS_WR_Cmd(cfg, SPI_MODE,temp);   //�趨ģʽ
}

//�������ȿ�/�����ú���.
//ս��V3������HT6872����,ͨ��VS1053��GPIO4(36��),�����乤��/�ر�.
//GPIO4=1,HT6872��������.
//GPIO4=0,HT6872�ر�(Ĭ��)
//sw:0,�ر�;1,����.
void VS_SPK_Set(vs10xx_cfg_t *cfg, u8 sw)
{
    VS_WRAM_Write(cfg, GPIO_DDR,1<<4);   //VS1053��GPIO4���ó����
    VS_WRAM_Write(cfg, GPIO_ODATA,sw<<4);//����VS1053��GPIO4���ֵ(0/1)
}

///////////////////////////////////////////////////////////////////////////////
//��������,��Ч��.

void VS_Set_All(vs10xx_cfg_t *cfg)
{
    VS_Set_Vol(cfg, vsset.mvol);         //��������
    VS_Set_Bass(cfg, vsset.bflimit,vsset.bass,vsset.tflimit,vsset.treble);
    VS_Set_Effect(cfg, vsset.effect);    //���ÿռ�Ч��
    VS_SPK_Set(cfg, vsset.speakersw);    //���ư�������״̬
}

//VS1053��WAV¼����bug,���plugin���������������
const u16 wav_plugin[40]=/* Compressed plugin */
{
    0x0007, 0x0001, 0x8010, 0x0006, 0x001c, 0x3e12, 0xb817, 0x3e14, /* 0 */
    0xf812, 0x3e01, 0xb811, 0x0007, 0x9717, 0x0020, 0xffd2, 0x0030, /* 8 */
    0x11d1, 0x3111, 0x8024, 0x3704, 0xc024, 0x3b81, 0x8024, 0x3101, /* 10 */
    0x8024, 0x3b81, 0x8024, 0x3f04, 0xc024, 0x2808, 0x4800, 0x36f1, /* 18 */
    0x9811, 0x0007, 0x0001, 0x8028, 0x0006, 0x0002, 0x2a00, 0x040e,
};
//����PCM ¼��ģʽ
//agc:0,�Զ�����.1024�൱��1��,512�൱��0.5��,���ֵ65535=64��

void recoder_enter_rec_mode(vs10xx_cfg_t *cfg, u16 agc)
{
    //�����IMA ADPCM,�����ʼ��㹫ʽ����:
    //������=CLKI/256*d;
    //����d=0,��2��Ƶ,�ⲿ����Ϊ12.288M.��ôFc=(2*12288000)/256*6=16Khz
    //���������PCM,������ֱ�Ӿ�д����ֵ
    VS_WR_Cmd(cfg, SPI_BASS,0x0000);
    VS_WR_Cmd(cfg, SPI_AICTRL0,8000);    //���ò�����,����Ϊ8Khz
    VS_WR_Cmd(cfg, SPI_AICTRL1,agc);     //��������,0,�Զ�����.1024�൱��1��,512�൱��0.5��,���ֵ65535=64��
    VS_WR_Cmd(cfg, SPI_AICTRL2,0);       //�����������ֵ,0,�������ֵ65536=64X
    VS_WR_Cmd(cfg, SPI_AICTRL3,6);       //��ͨ��(MIC����������)
    VS_WR_Cmd(cfg, SPI_CLOCKF,0X2000);   //����VS10XX��ʱ��,MULT:2��Ƶ;ADD:������;CLK:12.288Mhz
    VS_WR_Cmd(cfg, SPI_MODE,0x1804);     //MIC,¼������
    delay_ms(5);                    //�ȴ�����1.35ms
    VS_Load_Patch(cfg, (u16*)wav_plugin,40);//VS1053��WAV¼����Ҫpatch
}


