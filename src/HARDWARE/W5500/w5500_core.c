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
* 函数名  : Write_W5500_1Byte
* 描述    : 通过SPI1向指定地址寄存器写1个字节数据
* 输入    : reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_1Byte(w5500_cfg_t *cfg, unsigned short reg, unsigned char dat)
{
    cfg->W5500_SCS(RESET);                                  //置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);                         //通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_WRITE|COMMON_R);  //通过SPI1写控制字节,1个字节数据长度,写数据,选择通用寄存器
    cfg->W5500_SPI_ReadWriteByte(dat);                      //写1个字节数据

    cfg->W5500_SCS(SET);                                    //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_2Byte
* 描述    : 通过SPI1向指定地址寄存器写2个字节数据
* 输入    : reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_2Byte(w5500_cfg_t *cfg, unsigned short reg, unsigned short dat)
{
    cfg->W5500_SCS(RESET);                                          //置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);                                 //通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM2|RWB_WRITE|COMMON_R);          //通过SPI1写控制字节,2个字节数据长度,写数据,选择通用寄存器
    cfg->W5500_SPI_WriteShort(dat);                                 //写16位数据

    cfg->W5500_SCS(SET);                                            //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_nByte
* 描述    : 通过SPI1向指定地址寄存器写n个字节数据
* 输入    : reg:16位寄存器地址,*dat_ptr:待写入数据缓冲区指针,size:待写入的数据长度
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_nByte(w5500_cfg_t *cfg, unsigned short reg, unsigned char *dat_ptr, unsigned short size)
{
    unsigned short i;

    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(VDM|RWB_WRITE|COMMON_R);//通过SPI1写控制字节,N个字节数据长度,写数据,选择通用寄存器

    for(i=0; i<size; i++) //循环将缓冲区的size个字节数据写入W5500
    {
        cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//写一个字节数据
    }

    cfg->W5500_SCS(SET); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_1Byte
* 描述    : 通过SPI1向指定端口寄存器写1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:待写入的数据
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_1Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg, unsigned char dat)
{
    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,写数据,选择端口s的寄存器
    cfg->W5500_SPI_ReadWriteByte(dat);//写1个字节数据

    cfg->W5500_SCS(SET); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_2Byte
* 描述    : 通过SPI1向指定端口寄存器写2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,dat:16位待写入的数据(2个字节)
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_2Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg, unsigned short dat)
{
    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM2|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,写数据,选择端口s的寄存器
    cfg->W5500_SPI_WriteShort(dat);//写16位数据

    cfg->W5500_SCS(SET); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Write_W5500_SOCK_4Byte
* 描述    : 通过SPI1向指定端口寄存器写4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址,*dat_ptr:待写入的4个字节缓冲区指针
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void Write_W5500_SOCK_4Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg, unsigned char *dat_ptr)
{
    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM4|RWB_WRITE|(s*0x20+0x08));//通过SPI1写控制字节,4个字节数据长度,写数据,选择端口s的寄存器

    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//写第1个字节数据
    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//写第2个字节数据
    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//写第3个字节数据
    cfg->W5500_SPI_ReadWriteByte(*dat_ptr++);//写第4个字节数据

    cfg->W5500_SCS(SET); //置W5500的SCS为高电平
}

/*******************************************************************************
* 函数名  : Read_W5500_1Byte
* 描述    : 读W5500指定地址寄存器的1个字节数据
* 输入    : reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
void SPI1_Send_Byte(unsigned char dat)
{
    SPI_I2S_SendData(SPI2,dat);//D′1??×??úêy?Y
    while(SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);//μè′yêy?Y??′??÷??
}

unsigned char Read_W5500_1Byte(w5500_cfg_t *cfg, unsigned short reg)
{
    unsigned char i = 0;

    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_READ|COMMON_R);//通过SPI1写控制字节,1个字节数据长度,读数据,选择通用寄存器

    i = cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据

    cfg->W5500_SCS(SET);//置W5500的SCS为高电平


    return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_1Byte
* 描述    : 读W5500指定端口寄存器的1个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的1个字节数据
* 说明    : 无
*******************************************************************************/
unsigned char Read_W5500_SOCK_1Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg)
{
    unsigned char i;

    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM1|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,1个字节数据长度,读数据,选择端口s的寄存器

    i = cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据

    cfg->W5500_SCS(SET);//置W5500的SCS为高电平
    return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读W5500指定端口寄存器的2个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的2个字节数据(16位)
* 说明    : 无
*******************************************************************************/
unsigned short Read_W5500_SOCK_2Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg)
{
    unsigned short i;

    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM2|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,2个字节数据长度,读数据,选择端口s的寄存器

    i = cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据
    i*=256;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据

    cfg->W5500_SCS(SET);//置W5500的SCS为高电平
    return i;//返回读取到的寄存器数据
}

/*******************************************************************************
* 函数名  : Read_W5500_SOCK_2Byte
* 描述    : 读W5500指定端口寄存器的4个字节数据
* 输入    : s:端口号,reg:16位寄存器地址
* 输出    : 无
* 返回值  : 读取到寄存器的4个字节数据(16位)
* 说明    : 无
*******************************************************************************/
unsigned int Read_W5500_SOCK_4Byte(w5500_cfg_t *cfg, SOCKET s, unsigned short reg)
{
    unsigned int i;

    cfg->W5500_SCS(RESET);//置W5500的SCS为低电平

    cfg->W5500_SPI_WriteShort(reg);//通过SPI1写16位寄存器地址
    cfg->W5500_SPI_ReadWriteByte(FDM4|RWB_READ|(s*0x20+0x08));//通过SPI1写控制字节,4个字节数据长度,读数据,选择端口s的寄存器

    i= cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据
    i<<=8;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据
    i<<=8;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据
    i<<=8;
    i+=cfg->W5500_SPI_ReadWriteByte(0xff);//发送一个哑数据

    cfg->W5500_SCS(SET);//置W5500的SCS为高电平
    return i;//返回读取到的寄存器数据
}

u16 read_rx_buffer(SOCKET s, u8* data, u16 offset, u16 len)
{
    u32 i = 0;

    if((offset+len) < sn_rx_sz[s])//如果最大地址未超过W5500接收缓冲区寄存器的最大地址
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

        for(i = 0; i < offset; i++) //循环读取出前offset个字节数据
        {
            *data = s_w5500_cfgp->W5500_SPI_ReadWriteByte(0xff);
            data++;
        }
        s_w5500_cfgp->W5500_SCS(SET); //置W5500的SCS为高电平

        s_w5500_cfgp->W5500_SCS(RESET);//置W5500的SCS为低电平

        s_w5500_cfgp->W5500_SPI_WriteShort(0x00);//写16位地址
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_READ|(s*0x20+0x18));//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器

        for(; i < len; i++) //循环读取后rx_size-offset个字节数据
        {
            *data=s_w5500_cfgp->W5500_SPI_ReadWriteByte(0xff);//将读取到的数据保存到数据保存缓冲区

            data++;//数据保存缓冲区指针地址自增1
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
            goto exit;//没接收到数据则返回

        if(rx_size>808) rx_size=808;

        offset=Read_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RD);
        offset1=offset;
        offset&=(sn_rx_sz[s]-1);//计算实际的物理地址

        if((Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR) & 0x0f) == MR_UDP)
        {
            is_udp = 1;
        }
        else        //tcp get remote ip & port here
        {
            *remote_ip = Read_W5500_SOCK_4Byte(s_w5500_cfgp, s, Sn_DIPR);//读取远端主机IP
            *remote_ip = htonl(*remote_ip);
            *remote_port = Read_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_DPORTR);//读取远端主机端口号
            *remote_port = htons(*remote_port);
        }

        s_w5500_cfgp->W5500_SCS(RESET);//置W5500的SCS为低电平

        s_w5500_cfgp->W5500_SPI_WriteShort(offset);//写16位地址
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_READ|(s*0x20+0x18));//写控制字节,N个字节数据长度,读数据,选择端口s的寄存器

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
                s_w5500_cfgp->W5500_SCS(SET); //置W5500的SCS为高电平

                offset1+=rx_size;//更新实际物理地址,即下次读取接收到的数据的起始地址
                Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RD, offset1);
                Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, RECV);//发送启动接收命令

                goto exit;
            }
        }


        read_rx_buffer(s, dat_ptr, offset, rx_size);
        s_w5500_cfgp->W5500_SCS(SET); //置W5500的SCS为高电平

        offset1+=rx_size;//更新实际物理地址,即下次读取接收到的数据的起始地址
        Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_RX_RD, offset1);
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, RECV);//发送启动接收命令


        OSSemPost(sem_w5500);
    }

    return rx_size;//返回接收到数据的长度

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
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, dat);//发送主动断开连接命令
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
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_IR, dat);//发送主动断开连接命令
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

    if((offset+len)<sn_tx_sz[s])//如果最大地址未超过W5500发送缓冲区寄存器的最大地址
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
        for(i=0; i<len; i++) //循环写入size个字节数据
        {
            s_w5500_cfgp->W5500_SPI_ReadWriteByte(*data++);//写入一个字节的数据
        }
        //time_elapse = OSTimeGet() - time_stamp;

#endif
    }
    else        //如果最大地址超过W5500发送缓冲区寄存器的最大地址
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
        for(i=0; i<offset; i++) //循环写入前offset个字节数据
        {
            s_w5500_cfgp->W5500_SPI_ReadWriteByte(*data++);//写入一个字节的数据
        }
#endif
        s_w5500_cfgp->W5500_SCS(SET); //置W5500的SCS为高电平
        s_w5500_cfgp->W5500_SCS(RESET);//置W5500的SCS为低电平

        s_w5500_cfgp->W5500_SPI_WriteShort(0x00);//写16位地址
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_WRITE|(s*0x20+0x10));//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器
#if W5500_DMA_TX_ENABLE
        w5500_dma_enable(len-offset);
        /*
        do
        {
            i=DMA_GetCurrDataCounter(W5500_DMA_CHx);
        }
        while(i != 0);*/
#else
        for(; i<len; i++) //循环写入size-offset个字节数据
        {
            s_w5500_cfgp->W5500_SPI_ReadWriteByte(*data++);//写入一个字节的数据
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
            Write_W5500_SOCK_4Byte(s_w5500_cfgp, s, Sn_DIPR,   (unsigned char *)dst_ip);//设置目的主机IP
            Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_DPORTR, dst_port);//设置目的主机端口号
        }

        offset=Read_W5500_SOCK_2Byte(s_w5500_cfgp, s,Sn_TX_WR);
        offset1=offset;
        offset&=(sn_tx_sz[s]-1);//计算实际的物理地址

        s_w5500_cfgp->W5500_SCS(RESET);//置W5500的SCS为低电平

        s_w5500_cfgp->W5500_SPI_WriteShort(offset);//写16位地址
        s_w5500_cfgp->W5500_SPI_ReadWriteByte(VDM|RWB_WRITE|(s*0x20+0x10));//写控制字节,N个字节数据长度,写数据,选择端口s的寄存器

        write_tx_buffer(s, dat_ptr, offset, size);

        s_w5500_cfgp->W5500_SCS(SET); //置W5500的SCS为高电平

        offset1+=size;//更新实际物理地址,即下次写待发送数据到发送数据缓冲区的起始地址
        Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_TX_WR, offset1);
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, SEND);//发送启动发送命令

        OSSemPost(sem_w5500);
    }
    return 0;
}

/*******************************************************************************
* 函数名  : W5500_Hardware_Reset
* 描述    : 硬件复位W5500
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : W5500的复位引脚保持低电平至少500us以上,才能重围W5500
*******************************************************************************/
void W5500_Hardware_Reset(w5500_cfg_t *cfg)
{
    cfg->W5500_RST(RESET);//复位引脚拉低
    delay_ms(50);
    cfg->W5500_RST(SET);//复位引脚拉高
    delay_ms(200);
    while((Read_W5500_1Byte(cfg, PHYCFGR)&LINK)==0);//等待以太网连接完成
}

void W5500_Netcard_Setup(netchard_dev_t* dev)
{
    s_netchard_devp = dev;
}

/*******************************************************************************
* 函数名  : W5500_Init
* 描述    : 初始化W5500寄存器函数
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 在使用W5500之前，先对W5500初始化
*******************************************************************************/
void W5500_Init(w5500_cfg_t *cfg, netchard_dev_t* dev)
{
    u8 i=0;

    s_w5500_cfgp = cfg;
    W5500_Netcard_Setup(dev);

    Write_W5500_1Byte(cfg, MR, RST);        //软件复位W5500,置1有效,复位后自动清0
    delay_ms(10);                           //延时10ms,自己定义该函数

    //设置网关(Gateway)的IP地址,Gateway_IP为4字节unsigned char数组,自己定义
    //使用网关可以使通信突破子网的局限，通过网关可以访问到其它子网或进入Internet
    Write_W5500_nByte(cfg, GAR, s_netchard_devp->ip, 4);

    //设置子网掩码(MASK)值,SUB_MASK为4字节unsigned char数组,自己定义
    //子网掩码用于子网运算
    Write_W5500_nByte(cfg, SUBR,s_netchard_devp->netmask,4);

    //设置物理地址,PHY_ADDR为6字节unsigned char数组,自己定义,用于唯一标识网络设备的物理地址值
    //该地址值需要到IEEE申请，按照OUI的规定，前3个字节为厂商代码，后三个字节为产品序号
    //如果自己定义物理地址，注意第一个字节必须为偶数
    Write_W5500_nByte(cfg, SHAR,s_netchard_devp->mac,6);

    //设置本机的IP地址,IP_ADDR为4字节unsigned char数组,自己定义
    //注意，网关IP必须与本机IP属于同一个子网，否则本机将无法找到网关
    Write_W5500_nByte(cfg, SIPR,s_netchard_devp->ip,4);

    //设置发送缓冲区和接收缓冲区的大小，参考W5500数据手册

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

    //设置重试时间，默认为2000(200ms)
    //每一单位数值为100微秒,初始化时值设为2000(0x07D0),等于200毫秒
    Write_W5500_2Byte(cfg, RTR, 0x07d0);

    //设置重试次数，默认为8次
    //如果重发的次数超过设定值,则产生超时中断(相关的端口中断寄存器中的Sn_IR 超时位(TIMEOUT)置“1”)
    Write_W5500_1Byte(cfg, RCR,8);

}

/*******************************************************************************
* 函数名  : Socket_Init
* 描述    : 指定Socket(0~7)初始化
* 输入    : s:待初始化的端口
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/
void W5500_Socket_Init(SOCKET s, u16 port)
{
    //设置分片长度，参考W5500数据手册，该值可以不修改
    Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_MSSR, 1460);//最大分片字节数=1460(0x5b4)
    //设置指定端口
    Write_W5500_SOCK_2Byte(s_w5500_cfgp, s, Sn_PORT, port);
}

/*******************************************************************************
* 函数名  : Socket_Connect
* 描述    : 设置指定Socket(0~7)为客户端与远程服务器连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在客户端模式时,引用该程序,与远程服务器建立连接
*           如果启动连接后出现超时中断，则与服务器连接失败,需要重新调用该程序连接
*           该程序每调用一次,就与服务器产生一次连接
*******************************************************************************/
unsigned char W5500_Socket_Connect(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_TCP);         //设置socket为TCP模式
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);               //打开Socket
    delay_ms(5);                                        //延时5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR) != SOCK_INIT)  //如果socket打开失败
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);      //打开不成功,关闭Socket
        return FALSE;                                   //返回FALSE(0x00)
    }
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CONNECT);            //设置Socket为Connect模式
    return TRUE;                                        //返回TRUE,设置成功
}

/*******************************************************************************
* 函数名  : Socket_Listen
* 描述    : 设置指定Socket(0~7)作为服务器等待远程主机的连接
* 输入    : s:待设定的端口
* 输出    : 无
* 返回值  : 成功返回TRUE(0xFF),失败返回FALSE(0x00)
* 说明    : 当本机Socket工作在服务器模式时,引用该程序,等等远程主机的连接
*           该程序只调用一次,就使W5500设置为服务器模式
*******************************************************************************/
unsigned char W5500_Socket_Listen(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_TCP);//设置socket为TCP模式
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);//打开Socket
    delay_ms(5);//延时5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR)!=SOCK_INIT)//如果socket打开失败
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);//打开不成功,关闭Socket
        return FALSE;//返回FALSE(0x00)
    }
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, LISTEN);//设置Socket为侦听模式
    delay_ms(5);//延时5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR)!=SOCK_LISTEN)//如果socket设置失败
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);//设置不成功,关闭Socket
        return FALSE;//返回FALSE(0x00)
    }

    return TRUE;

    //至此完成了Socket的打开和设置侦听工作,至于远程客户端是否与它建立连接,则需要等待Socket中断，
    //以判断Socket的连接是否成功。参考W5500数据手册的Socket中断状态
    //在服务器侦听模式不需要设置目的IP和目的端口号
}

void W5500_Socket_UDP(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_UDP);//设置Socket为UDP模式*/
}

unsigned char W5500_Socket_UDP_Open(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, OPEN);//打开Socket*/
    delay_ms(5);//延时5ms
    if(Read_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_SR)!=SOCK_UDP)//如果Socket打开失败
    {
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_CR, CLOSE);//打开不成功,关闭Socket
        return FALSE;//返回FALSE(0x00)
    }
    else
        return TRUE;
}


void W5500_Socket_TCP(SOCKET s)
{
    Write_W5500_SOCK_1Byte(s_w5500_cfgp, s, Sn_MR, MR_TCP);//设置Socket为tcp模式*/

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
* 函数名  : W5500_Interrupt_Process
* 描述    : W5500中断处理程序框架
* 输入    : 无
* 输出    : 无
* 返回值  : 无
* 说明    : 无
*******************************************************************************/

void W5500_Interrupt_Process(void)
{
    unsigned char sir,sn_ir;

IntDispose:

    sir=Read_W5500_1Byte(s_w5500_cfgp, SIR);//读取端口中断标志寄存器
    if((sir & S0_INT) == S0_INT)//Socket0事件处理
    {
        sn_ir=Read_W5500_SOCK_1Byte(s_w5500_cfgp, 0, Sn_IR);//读取Socket0中断标志寄存器
        Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0,Sn_IR,sn_ir);
        if(sn_ir&IR_CON)//在TCP模式下,Socket0成功连接
        {
            //S0_State|=S_CONN;//网络连接状态0x02,端口完成连接，可以正常传输数据
        }
        if(sn_ir&IR_DISCON)//在TCP模式下Socket断开连接处理
        {
            Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0,Sn_CR,CLOSE);//关闭端口,等待重新打开连接
            //Socket_Init(0);       //指定Socket(0~7)初始化,初始化端口0
            //S0_State=0;//网络连接状态0x00,端口连接失败
        }
        if(sn_ir&IR_SEND_OK)//Socket0数据发送完成,可以再次启动S_tx_process()函数发送数据
        {
            sn_ir &= ~(1<<IR_SEND_OK);
            Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0, Sn_IR, sn_ir);
        }
        if(sn_ir&IR_RECV)//Socket接收到数据,可以启动S_rx_process()函数
        {
            ;//S0_Data|=S_RECEIVE;//端口接收到一个数据包
        }
        if(sn_ir&IR_TIMEOUT)//Socket连接或数据传输超时处理
        {
            ;//Write_W5500_SOCK_1Byte(s_w5500_cfgp, 0,Sn_CR,CLOSE);// 关闭端口,等待重新打开连接
            //S0_State=0;//网络连接状态0x00,端口连接失败
        }

        sir &= ~(1<<S0_INT);
        Write_W5500_1Byte(s_w5500_cfgp, SIR, sir);

    }

    if(Read_W5500_1Byte(s_w5500_cfgp, SIR) != 0)
        goto IntDispose;
}

