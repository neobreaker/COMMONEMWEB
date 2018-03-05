#include "stm32f10x.h"
#include "stm32f10x_spi.h"
#include "w5500_port.h"
#include "ucos_ii.h"

extern u8 w5500_send_buffer[2048];
extern OS_EVENT* sem_w5500_dma;

void w5500_dma_init(u32 cpar, u32 cmar)
{
    DMA_InitTypeDef DMA_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  //使能DMA传输

    DMA_DeInit(W5500_DMA_CHx);   //将DMA的通道1寄存器重设为缺省值

    DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA外设基地址
    DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA内存基地址
    DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //数据传输方向，从内存读取发送到外设
    DMA_InitStructure.DMA_BufferSize = 0;  //DMA通道的DMA缓存的大小
    DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //外设地址寄存器不变
    DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //内存地址寄存器递增
    DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //数据宽度为8位
    DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //数据宽度为8位
    DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //工作在正常模式
    DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMA通道 x拥有中优先级
    DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMA通道x没有设置为内存到内存传输

    DMA_Init(W5500_DMA_CHx, &DMA_InitStructure);  //根据DMA_InitStruct中指定的参数初始化DMA的通道USART1_Tx_DMA_Channel所标识的寄存器

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ITConfig(W5500_DMA_CHx, DMA_IT_TC, ENABLE);

}

//开启一次DMA传输
void w5500_dma_enable(u16 cndtr)
{
    u8 err;
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(W5500_DMA_CHx, DISABLE );               //关闭 所指示的通道
    DMA_SetCurrDataCounter(W5500_DMA_CHx, cndtr);   //DMA通道的DMA缓存的大小
    DMA_Cmd(W5500_DMA_CHx, ENABLE);                 //使能 所指示的通道
    OSSemPend(sem_w5500_dma, 0, &err);
}

void DMA1_Channel5_IRQHandler(void)
{
    if(DMA_GetITStatus(DMA1_IT_TC5)==SET)
    {
		OSSemPost(sem_w5500_dma);
        DMA_ClearITPendingBit(DMA1_IT_TC5);
    }
}

void SPI_Configuration(void)
{
    GPIO_InitTypeDef    GPIO_InitStructure;
    SPI_InitTypeDef     SPI_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOG | RCC_APB2Periph_AFIO, ENABLE);

    /* 初始化SCK、MISO、MOSI引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

    /* 初始化CS引脚 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    /* 初始化配置STM32 SPI2 */
    SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;    //SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode=SPI_Mode_Master;                         //设置为主SPI
    SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;                     //SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;                            //时钟悬空低
    SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;                          //数据捕获于第1个时钟沿
    SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;                             //NSS由外部管脚管理
    SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;    //波特率预分频值为2
    SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;                    //数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial=7;                              //CRC多项式为7
    SPI_Init(SPI2,&SPI_InitStructure);                                  //根据SPI_InitStruct中指定的参数初始化外设SPI1寄存器

    SPI_Cmd(SPI2,ENABLE);   //STM32使能SPI1
}

void W5500_GPIO_Configuration(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG , ENABLE);
    /* W5500_RST引脚初始化配置*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOG, GPIO_Pin_6);

    SPI_Configuration();
    w5500_dma_init((u32)&SPI2->DR, (u32)w5500_send_buffer);
}

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI2_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        retry++;
        if(retry>200)return 0;
    }
    SPI_I2S_SendData(SPI2, TxData); //通过外设SPIx发送一个数据
    retry=0;

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        retry++;
        if(retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI2); //返回通过SPIx最近接收的数据
}

void SPI2_WriteShort(unsigned short dat)
{
    u8 *p = (u8*)&dat;
    SPI2_ReadWriteByte(*(p+1));     //写数据高位
    SPI2_ReadWriteByte(*p);         //写数据低位
}

void w5500_rst(u8 stat)
{
    PGout(6) = stat;
}

void w5500_scs(u8 stat)
{
    PBout(12) = stat;
}

void w5500_cfg_setup(w5500_cfg_t* cfg)
{
    cfg->W5500_Init                 = W5500_GPIO_Configuration;
    cfg->W5500_SPI_ReadWriteByte    = SPI2_ReadWriteByte;
    cfg->W5500_SPI_WriteShort       = SPI2_WriteShort;
    cfg->W5500_RST                  = w5500_rst;
    cfg->W5500_SCS                  = w5500_scs;

    if(cfg->W5500_Init)
        cfg->W5500_Init();
}

