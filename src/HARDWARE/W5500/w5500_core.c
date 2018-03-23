#include "stm32f10x.h"
#include "w5500_core.h"
#include "w5500_socket.h"
#include "w5500_port.h"
#include "ucos_ii.h"

extern OS_EVENT* sem_w5500;

static w5500_cfg_t* s_w5500_cfgp;
static netchard_dev_t* s_netchard_devp;
static u16 sn_tx_sz[8];
static u16 sn_rx_sz[8];

/*******************************************************************************
* ������  : Write_W5500_1Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_1Byte(w5500_cfg_t *cfg, unsigned short reg, unsigned char dat)
{
    cfg->W5500_SCS(RESET);                                  //��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);                         //ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_WRITE|COMMON_R);  //ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
    cfg->W5500_SPI_ReadWriteByte(dat);                      //д1���ֽ�����

    cfg->W5500_SCS(SET);                                    //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_2Byte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���д2���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_2Byte(w5500_cfg_t *cfg, unsigned short reg, unsigned short dat)
{
    cfg->W5500_SCS(RESET);                                          //��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);                                 //ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM2|RWB_WRITE|COMMON_R);          //ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���
    cfg->W5500_SPI_WriteShort(dat);                                 //д16λ����

    cfg->W5500_SCS(SET);                                            //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_nByte
* ����    : ͨ��SPI1��ָ����ַ�Ĵ���дn���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ,*dat_ptr:��д�����ݻ�����ָ��,size:��д������ݳ���
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_nByte(w5500_cfg_t *cfg, unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
    unsigned short i;

    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(VDM|RWB_WRITE|COMMON_R);//ͨ��SPI1д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��ͨ�üĴ���

    for(i=0; i<size; i++) //ѭ������������size���ֽ�����д��W5500
    {
        cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//дһ���ֽ�����
    }

    cfg->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_1Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:��д�������
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_1Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg, unsigned char dat)
{
    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_WRITE|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
    cfg->W5500_SPI_ReadWriteByte(dat);//д1���ֽ�����

    cfg->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_2Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,dat:16λ��д�������(2���ֽ�)
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_2Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg, unsigned short dat)
{
    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM2|RWB_WRITE|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
    cfg->W5500_SPI_WriteShort(dat);//д16λ����

    cfg->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Write_W5500_SOCK_4Byte
* ����    : ͨ��SPI1��ָ���˿ڼĴ���д4���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ,*dat_ptr:��д���4���ֽڻ�����ָ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void Write_W5500_SOCK_4Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM4|RWB_WRITE|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,4���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//д��1���ֽ�����
    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//д��2���ֽ�����
    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//д��3���ֽ�����
    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//д��4���ֽ�����

    cfg->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ
}

/*******************************************************************************
* ������  : Read_W5500_1Byte
* ����    : ��W5500ָ����ַ�Ĵ�����1���ֽ�����
* ����    : reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
*******************************************************************************/
void SPI1_Send_Byte(unsigned char dat)
{
    SPI_I2S_SendData(SPI2,dat);//D��1??��??����y?Y
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);//�̨���y��y?Y??��??��??
}

unsigned char Read_W5500_1Byte(w5500_cfg_t *cfg, unsigned short reg)
{
    unsigned char i = 0;

    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_READ|COMMON_R);//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,������,ѡ��ͨ�üĴ���

    i = cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������

    cfg->W5500_SCS(SET);//��W5500��SCSΪ�ߵ�ƽ


    return i;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_1Byte
* ����    : ��W5500ָ���˿ڼĴ�����1���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����1���ֽ�����
* ˵��    : ��
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg)
{
    unsigned char i;

    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_READ|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,1���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

    i = cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������

    cfg->W5500_SCS(SET);//��W5500��SCSΪ�ߵ�ƽ
    return i;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_2Byte
* ����    : ��W5500ָ���˿ڼĴ�����2���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����2���ֽ�����(16λ)
* ˵��    : ��
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg)
{
    unsigned short i;

    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM2|RWB_READ|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,2���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

    i = cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������
    i*=256;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������

    cfg->W5500_SCS(SET);//��W5500��SCSΪ�ߵ�ƽ
    return i;//���ض�ȡ���ļĴ�������
}

/*******************************************************************************
* ������  : Read_W5500_SOCK_2Byte
* ����    : ��W5500ָ���˿ڼĴ�����4���ֽ�����
* ����    : s:�˿ں�,reg:16λ�Ĵ�����ַ
* ���    : ��
* ����ֵ  : ��ȡ���Ĵ�����4���ֽ�����(16λ)
* ˵��    : ��
*******************************************************************************/
unsigned int Read_W5500_SOCK_4Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg)
{
    unsigned int i;

    cfg->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

    cfg->W5500_SPI_WriteShort(reg);//ͨ��SPI1д16λ�Ĵ�����ַ
    cfg->W5500_SPI_ReadWriteByte(FDM4|RWB_READ|(s*0x20+0x08));//ͨ��SPI1д�����ֽ�,4���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

    i= cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������
    i<<=8;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������
    i<<=8;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������
    i<<=8;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//����һ��������

    cfg->W5500_SCS(SET);//��W5500��SCSΪ�ߵ�ƽ
    return i;//���ض�ȡ���ļĴ�������
}

u16 read_rx_buffer(SOCKET s, u8* data, u16 offset, u16 len)
{
    u32 i = 0;

    if((offset+len) < sn_rx_sz[s])//�������ַδ����W5500���ջ������Ĵ���������ַ
    {
        for(i = 0; i < len; i++)
        {
            *data = s_w5500_cfgp->W5500_SPI_ReadWriteByte(0xff);
            data++;
        }
    }
    else
    {
        offset = sn_rx_sz[s] - offset;

        for(i = 0; i < offset; i++) //ѭ����ȡ��ǰoffset���ֽ�����
        {
            *data = s_w5500_cfgp->W5500_SPI_ReadWriteByte(0xff);
            data++;
        }
        s_w5500_cfgp->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ

        s_w5500_cfgp->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

        s_w5500_cfgp->W5500_SPI_WriteShort(0x00);//д16λ��ַ
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_READ|(s*0x20+0x18));//д�����ֽ�,N���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

        for(; i < len; i++) //ѭ����ȡ��rx_size-offset���ֽ�����
        {
            *data=s_w5500_cfgp->W5500_SPI_ReadWriteByte(0xff);//����ȡ�������ݱ��浽���ݱ��滺����

            data++;//���ݱ��滺����ָ���ַ����1
        }
    }

    return len;
}

unsigned short Read_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, u32* remote_ip, u16* remote_port)
{
    unsigned short rx_size;
    unsigned short offset, offset1;
    udp_header_t udp_hd;
    u8 is_udp = 0;
    u8 err;

    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        rx_size=Read_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RSR);
        if(rx_size==0)
            goto exit;//û���յ������򷵻�

        if(rx_size>808) rx_size=808;

        offset=Read_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RD);
        offset1=offset;
        offset&=(sn_rx_sz[s]-1);//����ʵ�ʵ������ַ

        if((Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR) & 0x0f) == MR_UDP)
        {
            is_udp = 1;
        }
        else        //tcp get remote ip & port here
        {
            *remote_ip = Read_W5500_SOCK_4Byte(s_w5500_cfgp, s, Sn_DIPR);//��ȡԶ������IP
            *remote_ip = htonl(*remote_ip);
            *remote_port = Read_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_DPORTR);//��ȡԶ�������˿ں�
            *remote_port = htons(*remote_port);
        }

        s_w5500_cfgp->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

        s_w5500_cfgp->W5500_SPI_WriteShort(offset);//д16λ��ַ
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_READ|(s*0x20+0x18));//д�����ֽ�,N���ֽ����ݳ���,������,ѡ��˿�s�ļĴ���

        if(is_udp)
        {
            //if(rx_size>808) rx_size=808;

            //udp frame has 8 bytes header
            read_rx_buffer(s, (u8*)&udp_hd, offset, 8);
            udp_hd.size = ntohs(udp_hd.size);
            *remote_port = udp_hd.port;
            *remote_ip = udp_hd.ip;

            offset += 8;

            if(udp_hd.size + 8 == rx_size)
            {
                rx_size = udp_hd.size;      //if udp; remove 8 bytes header
                offset1 += 8;       //rx_size -= 8
            }
            else        //err occur
            {
                read_rx_buffer(s, dat_ptr, offset, rx_size-8);
                s_w5500_cfgp->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ

                offset1+=rx_size;//����ʵ�������ַ,���´ζ�ȡ���յ������ݵ���ʼ��ַ
                Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RD, offset1);
                Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, RECV);//����������������

                goto exit;
            }
        }


        read_rx_buffer(s, dat_ptr, offset, rx_size);
        s_w5500_cfgp->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ

        offset1+=rx_size;//����ʵ�������ַ,���´ζ�ȡ���յ������ݵ���ʼ��ַ
        Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RD, offset1);
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, RECV);//����������������


        OSSemPost(sem_w5500);
    }

    return rx_size;//���ؽ��յ����ݵĳ���

exit:
    OSSemPost(sem_w5500);
    return 0;
}

u8 getSn_SR(int s)
{
    u8 err;
    u8 ret;

    if(s < 0 || s > W5500_SOCKET_NUM)
        return SOCK_CLOSED;
    
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        ret = Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR);
    }
    OSSemPost(sem_w5500);

    return ret;
}

uint16_t getSn_RX_RSR(uint8_t s)
{
    u16 ret;
    u8 err;
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        Read_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RSR);
    }
    OSSemPost(sem_w5500);

    return ret;
}

u8 getSn_CR(int s)
{
    u8 err;
    u8 ret;

    if(s < 0 || s > W5500_SOCKET_NUM)
        return SOCK_CLOSED;
    
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        ret = Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR);
    }
    OSSemPost(sem_w5500);

    return ret;
}


u8 setSn_CR(int s, u8 dat)
{
    u8 err;
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, dat);//���������Ͽ���������
    }
    OSSemPost(sem_w5500);

    return 0;
}

u8 setSn_IR(int s, u8 dat)
{
    u8 err;
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_IR, dat);//���������Ͽ���������
    }
    OSSemPost(sem_w5500);

    return 0;
}

u8 setSn_DIPR(int s, u8* dat)
{
    u8 err;
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        Write_W5500_SOCK_4Byte(s_w5500_cfgp, s, Sn_DIPR, dat);
    }
    OSSemPost(sem_w5500);

    return 0;
}

u8 setSn_DPORTR(int s, u16 dat)
{
    u8 err;
    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
		Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_DPORTR, dat);
    }
    OSSemPost(sem_w5500);

    return 0;
}

//static u32 time_stamp = 0, time_elapse = 0;
int write_tx_buffer(SOCKET s, unsigned char *data, unsigned short offset, unsigned short len)
{
    unsigned short i;

    if((offset+len)<sn_tx_sz[s])//�������ַδ����W5500���ͻ������Ĵ���������ַ
    {

#if W5500_DMA_TX_ENABLE

        //time_stamp = OSTimeGet();

        w5500_dma_enable(len);

        //time_elapse = OSTimeGet() - time_stamp;

        /*do
        {
            i=DMA_GetCurrDataCounter(W5500_DMA_CHx);
        }
        while(i != 0);
        */
#else

        //time_stamp = OSTimeGet();
        for(i=0; i<len; i++) //ѭ��д��size���ֽ�����
        {
            s_w5500_cfgp->W5500_SPI_ReadWriteByte(*data++);//д��һ���ֽڵ�����
        }
        //time_elapse = OSTimeGet() - time_stamp;

#endif
    }
    else        //�������ַ����W5500���ͻ������Ĵ���������ַ
    {
        offset=sn_tx_sz[s]-offset;
#if W5500_DMA_TX_ENABLE
        w5500_dma_enable(offset);
        /*
        do
        {
            i=DMA_GetCurrDataCounter(W5500_DMA_CHx);
        }
        while(i != 0);*/
#else
        for(i=0; i<offset; i++) //ѭ��д��ǰoffset���ֽ�����
        {
            s_w5500_cfgp->W5500_SPI_ReadWriteByte(*data++);//д��һ���ֽڵ�����
        }
#endif
        s_w5500_cfgp->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ
        s_w5500_cfgp->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

        s_w5500_cfgp->W5500_SPI_WriteShort(0x00);//д16λ��ַ
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_WRITE|(s*0x20+0x10));//д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���
#if W5500_DMA_TX_ENABLE
        w5500_dma_enable(len-offset);
        /*
        do
        {
            i=DMA_GetCurrDataCounter(W5500_DMA_CHx);
        }
        while(i != 0);*/
#else
        for(; i<len; i++) //ѭ��д��size-offset���ֽ�����
        {
            s_w5500_cfgp->W5500_SPI_ReadWriteByte(*data++);//д��һ���ֽڵ�����
        }
#endif
    }

    return len;
}

int Write_SOCK_Data_Buffer(SOCKET s, unsigned char *dat_ptr, unsigned short size, unsigned char *dst_ip, unsigned short dst_port)
{
    unsigned short offset,offset1;

    u8 err;

    OSSemPend(sem_w5500, 0, &err);
    if(err == OS_ERR_NONE)
    {
        if((Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR)&0x0f) == MR_UDP)
        {
            Write_W5500_SOCK_4Byte(s_w5500_cfgp, s, Sn_DIPR,   (unsigned char *)dst_ip);//����Ŀ������IP
            Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_DPORTR, dst_port);//����Ŀ�������˿ں�
        }

        offset=Read_W5500_SOCK_2Byte(s_w5500_cfgp, s,Sn_TX_WR);
        offset1=offset;
        offset&=(sn_tx_sz[s]-1);//����ʵ�ʵ������ַ

        s_w5500_cfgp->W5500_SCS(RESET);//��W5500��SCSΪ�͵�ƽ

        s_w5500_cfgp->W5500_SPI_WriteShort(offset);//д16λ��ַ
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_WRITE|(s*0x20+0x10));//д�����ֽ�,N���ֽ����ݳ���,д����,ѡ��˿�s�ļĴ���

        write_tx_buffer(s, dat_ptr, offset, size);

        s_w5500_cfgp->W5500_SCS(SET); //��W5500��SCSΪ�ߵ�ƽ

        offset1+=size;//����ʵ�������ַ,���´�д���������ݵ��������ݻ���������ʼ��ַ
        Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_TX_WR, offset1);
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, SEND);//����������������

        OSSemPost(sem_w5500);
    }
    return 0;
}

/*******************************************************************************
* ������  : W5500_Hardware_Reset
* ����    : Ӳ����λW5500
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : W5500�ĸ�λ���ű��ֵ͵�ƽ����500us����,������ΧW5500
*******************************************************************************/
void W5500_Hardware_Reset(w5500_cfg_t *cfg)
{
    cfg->W5500_RST(RESET);//��λ��������
    delay_ms(50);
    cfg->W5500_RST(SET);//��λ��������
    delay_ms(200);
    while((Read_W5500_1Byte(cfg, PHYCFGR)&LINK)==0);//�ȴ���̫���������
}

void W5500_Netcard_Setup(netchard_dev_t* dev)
{
    s_netchard_devp = dev;
}

/*******************************************************************************
* ������  : W5500_Init
* ����    : ��ʼ��W5500�Ĵ�������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��ʹ��W5500֮ǰ���ȶ�W5500��ʼ��
*******************************************************************************/
void W5500_Init(w5500_cfg_t *cfg, netchard_dev_t* dev)
{
    u8 i=0;

    s_w5500_cfgp = cfg;
    W5500_Netcard_Setup(dev);

    Write_W5500_1Byte(cfg, MR, RST);        //�����λW5500,��1��Ч,��λ���Զ���0
    delay_ms(10);                           //��ʱ10ms,�Լ�����ú���

    //��������(Gateway)��IP��ַ,Gateway_IPΪ4�ֽ�unsigned char����,�Լ�����
    //ʹ�����ؿ���ʹͨ��ͻ�������ľ��ޣ�ͨ�����ؿ��Է��ʵ��������������Internet
    Write_W5500_nByte(cfg, GAR, s_netchard_devp->ip, 4);

    //������������(MASK)ֵ,SUB_MASKΪ4�ֽ�unsigned char����,�Լ�����
    //��������������������
    Write_W5500_nByte(cfg, SUBR,s_netchard_devp->netmask,4);

    //���������ַ,PHY_ADDRΪ6�ֽ�unsigned char����,�Լ�����,����Ψһ��ʶ�����豸�������ֵַ
    //�õ�ֵַ��Ҫ��IEEE���룬����OUI�Ĺ涨��ǰ3���ֽ�Ϊ���̴��룬�������ֽ�Ϊ��Ʒ���
    //����Լ����������ַ��ע���һ���ֽڱ���Ϊż��
    Write_W5500_nByte(cfg, SHAR,s_netchard_devp->mac,6);

    //���ñ�����IP��ַ,IP_ADDRΪ4�ֽ�unsigned char����,�Լ�����
    //ע�⣬����IP�����뱾��IP����ͬһ�����������򱾻����޷��ҵ�����
    Write_W5500_nByte(cfg, SIPR,s_netchard_devp->ip,4);

    //���÷��ͻ������ͽ��ջ������Ĵ�С���ο�W5500�����ֲ�

    /*
        Write_W5500_SOCK_1Byte(cfg, 0, Sn_RXBUF_SIZE, 0x08);
        Write_W5500_SOCK_1Byte(cfg, 0, Sn_TXBUF_SIZE, 0x08);
        Write_W5500_SOCK_1Byte(cfg, 1, Sn_RXBUF_SIZE, 0x08);
        Write_W5500_SOCK_1Byte(cfg, 1, Sn_TXBUF_SIZE, 0x08);
        sn_tx_sz[0] = 0x2000;
        sn_rx_sz[0] = 0x2000;
        sn_tx_sz[1] = 0x2000;
        sn_rx_sz[1] = 0x2000;
    */
    for(i = 0; i < 4; i++)
    {
        Write_W5500_SOCK_1Byte(cfg, i, Sn_RXBUF_SIZE, 0x04);//Socket Rx memory size=2k
        Write_W5500_SOCK_1Byte(cfg, i, Sn_TXBUF_SIZE, 0x04);//Socket Tx mempry size=2k

        sn_tx_sz[i] = 4096;
        sn_rx_sz[i] = 4096;
    }
    for(; i < 8; i++)
    {
        Write_W5500_SOCK_1Byte(cfg, i, Sn_RXBUF_SIZE, 0x00);//Socket Rx memory size=2k
        Write_W5500_SOCK_1Byte(cfg, i, Sn_TXBUF_SIZE, 0x00);//Socket Tx mempry size=2k

        sn_tx_sz[i] = 0;
        sn_rx_sz[i] = 0;
    }

    //��������ʱ�䣬Ĭ��Ϊ2000(200ms)
    //ÿһ��λ��ֵΪ100΢��,��ʼ��ʱֵ��Ϊ2000(0x07D0),����200����
    Write_W5500_2Byte(cfg, RTR, 0x07d0);

    //�������Դ�����Ĭ��Ϊ8��
    //����ط��Ĵ��������趨ֵ,�������ʱ�ж�(��صĶ˿��жϼĴ����е�Sn_IR ��ʱλ(TIMEOUT)�á�1��)
    Write_W5500_1Byte(cfg, RCR,8);

}

/*******************************************************************************
* ������  : Socket_Init
* ����    : ָ��Socket(0~7)��ʼ��
* ����    : s:����ʼ���Ķ˿�
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/
void W5500_Socket_Init(SOCKET s, u16 port)
{
    //���÷�Ƭ���ȣ��ο�W5500�����ֲᣬ��ֵ���Բ��޸�
    Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_MSSR, 1460);//����Ƭ�ֽ���=1460(0x5b4)
    //����ָ���˿�
    Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_PORT, port);
}

/*******************************************************************************
* ������  : Socket_Connect
* ����    : ����ָ��Socket(0~7)Ϊ�ͻ�����Զ�̷���������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڿͻ���ģʽʱ,���øó���,��Զ�̷�������������
*           ����������Ӻ���ֳ�ʱ�жϣ��������������ʧ��,��Ҫ���µ��øó�������
*           �ó���ÿ����һ��,�������������һ������
*******************************************************************************/
unsigned char W5500_Socket_Connect(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_TCP);         //����socketΪTCPģʽ
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);               //��Socket
    delay_ms(5);                                        //��ʱ5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR) != SOCK_INIT)  //���socket��ʧ��
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);      //�򿪲��ɹ�,�ر�Socket
        return FALSE;                                   //����FALSE(0x00)
    }
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CONNECT);            //����SocketΪConnectģʽ
    return TRUE;                                        //����TRUE,���óɹ�
}

/*******************************************************************************
* ������  : Socket_Listen
* ����    : ����ָ��Socket(0~7)��Ϊ�������ȴ�Զ������������
* ����    : s:���趨�Ķ˿�
* ���    : ��
* ����ֵ  : �ɹ�����TRUE(0xFF),ʧ�ܷ���FALSE(0x00)
* ˵��    : ������Socket�����ڷ�����ģʽʱ,���øó���,�ȵ�Զ������������
*           �ó���ֻ����һ��,��ʹW5500����Ϊ������ģʽ
*******************************************************************************/
unsigned char W5500_Socket_Listen(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_TCP);//����socketΪTCPģʽ
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);//��Socket
    delay_ms(5);//��ʱ5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR)!=SOCK_INIT)//���socket��ʧ��
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);//�򿪲��ɹ�,�ر�Socket
        return FALSE;//����FALSE(0x00)
    }
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, LISTEN);//����SocketΪ����ģʽ
    delay_ms(5);//��ʱ5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR)!=SOCK_LISTEN)//���socket����ʧ��
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);//���ò��ɹ�,�ر�Socket
        return FALSE;//����FALSE(0x00)
    }

    return TRUE;

    //���������Socket�Ĵ򿪺�������������,����Զ�̿ͻ����Ƿ�������������,����Ҫ�ȴ�Socket�жϣ�
    //���ж�Socket�������Ƿ�ɹ����ο�W5500�����ֲ��Socket�ж�״̬
    //�ڷ���������ģʽ����Ҫ����Ŀ��IP��Ŀ�Ķ˿ں�
}

void W5500_Socket_UDP(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_UDP);//����SocketΪUDPģʽ*/
}

unsigned char W5500_Socket_UDP_Open(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);//��Socket*/
    delay_ms(5);//��ʱ5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR)!=SOCK_UDP)//���Socket��ʧ��
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);//�򿪲��ɹ�,�ر�Socket
        return FALSE;//����FALSE(0x00)
    }
    else
        return TRUE;
}


void W5500_Socket_TCP(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_TCP);//����SocketΪtcpģʽ*/

}

unsigned char W5500_Socket_TCP_Open(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);

    delay_ms(5);
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR) != SOCK_INIT)
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);
        return FALSE;
    }
    return TRUE;
}

unsigned char W5500_Socket_TCP_Listen(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, LISTEN);

    delay_ms(5);
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR) != SOCK_LISTEN)
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);
        return FALSE;
    }
    return TRUE;
}

unsigned char W5500_Socket_TCP_Connect(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s,Sn_CR,CONNECT);

    return TRUE;
}

/*******************************************************************************
* ������  : W5500_Interrupt_Process
* ����    : W5500�жϴ��������
* ����    : ��
* ���    : ��
* ����ֵ  : ��
* ˵��    : ��
*******************************************************************************/

void W5500_Interrupt_Process(void)
{
    unsigned char sir,sn_ir;

IntDispose:

    sir=Read_W5500_1Byte(s_w5500_cfgp, SIR);//��ȡ�˿��жϱ�־�Ĵ���
    if((sir & S0_INT) == S0_INT)//Socket0�¼�����
    {
        sn_ir=Read_W5500_SOCK_1Byte(s_w5500_cfgp, 0, Sn_IR);//��ȡSocket0�жϱ�־�Ĵ���
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0,Sn_IR,sn_ir);
        if(sn_ir&IR_CON)//��TCPģʽ��,Socket0�ɹ�����
        {
            //S0_State|=S_CONN;//��������״̬0x02,�˿�������ӣ�����������������
        }
        if(sn_ir&IR_DISCON)//��TCPģʽ��Socket�Ͽ����Ӵ���
        {
            Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0,Sn_CR,CLOSE);//�رն˿�,�ȴ����´�����
            //Socket_Init(0);       //ָ��Socket(0~7)��ʼ��,��ʼ���˿�0
            //S0_State=0;//��������״̬0x00,�˿�����ʧ��
        }
        if(sn_ir&IR_SEND_OK)//Socket0���ݷ������,�����ٴ�����S_tx_process()������������
        {
            sn_ir &= ~(1<<IR_SEND_OK);
            Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0, Sn_IR, sn_ir);
        }
        if(sn_ir&IR_RECV)//Socket���յ�����,��������S_rx_process()����
        {
            ;//S0_Data|=S_RECEIVE;//�˿ڽ��յ�һ�����ݰ�
        }
        if(sn_ir&IR_TIMEOUT)//Socket���ӻ����ݴ��䳬ʱ����
        {
            ;//Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0,Sn_CR,CLOSE);// �رն˿�,�ȴ����´�����
            //S0_State=0;//��������״̬0x00,�˿�����ʧ��
        }

        sir &= ~(1<<S0_INT);
        Write_W5500_1Byte(s_w5500_cfgp, SIR, sir);

    }

    if(Read_W5500_1Byte(s_w5500_cfgp, SIR) != 0)
        goto IntDispose;
}

