#ifndef __VS10XX_CORE_H__
#define __VS10XX_CORE_H__

#include "stm32f10x.h"

#define SEND_NUM_PER_FRAME			160

typedef struct
{
    void (*VS_Init)(void);
    u8   (*VS_SPI_ReadWriteByte)(u8 TxData);
    void (*VS_SPI_SpeedLow)(void);
    void (*VS_SPI_SpeedHigh)(void);
    u8   (*VS_WAIT_DQ)(void);
    u8   (*VS_DQ)(void);
    void (*VS_RST)(u8 stat);
    void (*VS_XCS)(u8 stat);
    void (*VS_XDCS)(u8 stat);

} vs10xx_cfg_t;

__packed typedef struct
{
    u8 mvol;        //������,��Χ:0~254
    u8 bflimit;     //����Ƶ���޶�,��Χ:2~15(��λ:10Hz)
    u8 bass;        //����,��Χ:0~15.0��ʾ�ر�.(��λ:1dB)
    u8 tflimit;     //����Ƶ���޶�,��Χ:1~15(��λ:Khz)
    u8 treble;      //����,��Χ:0~15(��λ:1.5dB)(ԭ����Χ��:-8~7,ͨ�������޸���);
    u8 effect;      //�ռ�Ч������.0,�ر�;1,��С;2,�е�;3,���.
    u8 speakersw;   //�������ȿ���,0,�ر�;1,��
    u8 saveflag;    //�����־,0X0A,�������;����,����δ����
} _vs10xx_obj;


extern _vs10xx_obj vsset;       //VS10XX����

#define VS_WRITE_COMMAND    0x02
#define VS_READ_COMMAND     0x03
//VS10XX�Ĵ�������
#define SPI_MODE            0x00
#define SPI_STATUS          0x01
#define SPI_BASS            0x02
#define SPI_CLOCKF          0x03
#define SPI_DECODE_TIME     0x04
#define SPI_AUDATA          0x05
#define SPI_WRAM            0x06
#define SPI_WRAMADDR        0x07
#define SPI_HDAT0           0x08
#define SPI_HDAT1           0x09

#define SPI_AIADDR          0x0a
#define SPI_VOL             0x0b
#define SPI_AICTRL0         0x0c
#define SPI_AICTRL1         0x0d
#define SPI_AICTRL2         0x0e
#define SPI_AICTRL3         0x0f
#define SM_DIFF             0x01
#define SM_JUMP             0x02
#define SM_RESET            0x04
#define SM_OUTOFWAV         0x08
#define SM_PDOWN            0x10
#define SM_TESTS            0x20
#define SM_STREAM           0x40
#define SM_PLUSV            0x80
#define SM_DACT             0x100
#define SM_SDIORD           0x200
#define SM_SDISHARE         0x400
#define SM_SDINEW           0x800
#define SM_ADPCM            0x1000
#define SM_ADPCM_HP         0x2000

#define I2S_CONFIG          0XC040
#define GPIO_DDR            0XC017
#define GPIO_IDATA          0XC018
#define GPIO_ODATA          0XC019


void VS_Soft_Reset(vs10xx_cfg_t *cfg);
u8 VS_HD_Reset(vs10xx_cfg_t *cfg);
void VS_Sine_Test(vs10xx_cfg_t *cfg);
u16 VS_Ram_Test(vs10xx_cfg_t *cfg);
void VS_WR_Cmd(vs10xx_cfg_t *cfg, u8 address,u16 data);
void VS_WR_Data(vs10xx_cfg_t *cfg, u8 data);
u16 VS_RD_Reg(vs10xx_cfg_t *cfg, u8 address);
u16 VS_WRAM_Read(vs10xx_cfg_t *cfg, u16 addr);
void VS_WRAM_Write(vs10xx_cfg_t *cfg, u16 addr,u16 val);
void VS_Set_Speed(vs10xx_cfg_t *cfg, u8 t);
u16 VS_Get_HeadInfo(vs10xx_cfg_t *cfg);
u32 VS_Get_ByteRate(vs10xx_cfg_t *cfg);
u16 VS_Get_EndFillByte(vs10xx_cfg_t *cfg);
u8 VS_Send_MusicData(vs10xx_cfg_t *cfg, u8* buf);
u8 VS_Send_MusicData2(vs10xx_cfg_t *cfg, u8* buf, int len);
void VS_Restart_Play(vs10xx_cfg_t *cfg);
void VS_Reset_DecodeTime(vs10xx_cfg_t *cfg);
u16 VS_Get_DecodeTime(vs10xx_cfg_t *cfg);
void VS_Load_Patch(vs10xx_cfg_t *cfg, u16 *patch,u16 len);
void VS_Set_Vol(vs10xx_cfg_t *cfg, u8 volx);
void VS_Set_Bass(vs10xx_cfg_t *cfg, u8 bfreq,u8 bass,u8 tfreq,u8 treble);
void VS_Set_Effect(vs10xx_cfg_t *cfg, u8 eft);
void VS_SPK_Set(vs10xx_cfg_t *cfg, u8 sw);
void VS_Set_All(vs10xx_cfg_t *cfg);

void recoder_enter_rec_mode(vs10xx_cfg_t *cfg, u16 agc);

#endif
