#include "vs10xx_play_port.h"
#include "mysys.h"

extern u8 vs10xx_play_dma_buff[SEND_NUM_PER_FRAME];

u8 SPI3_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_TXE) == RESET)
    {
        retry++;
        if(retry>200)return 0;
    }
    SPI_I2S_SendData(SPI3, TxData);
    retry=0;

    while (SPI_I2S_GetFlagStatus(SPI3, SPI_I2S_FLAG_RXNE) == RESET)
    {
        retry++;
        if(retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI3);
}

void SPI3_SetSpeed(u8 SpeedSet)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    SPI3->CR1&=0XFFC7;
    SPI3->CR1|=SpeedSet;
    SPI_Cmd(SPI3,ENABLE);
}

void vs10xx_play_spi_speedlow(void)
{
    SPI3_SetSpeed(SPI_BaudRatePrescaler_32);
}

void vs10xx_play_spi_speedhigh(void)
{
    SPI3_SetSpeed(SPI_BaudRatePrescaler_8);
}

u8 vs10xx_play_dq()
{
    return PCin(1);
}

void vs10xx_play_rst(u8 stat)
{
    PCout(0) = stat;
}

void vs10xx_play_xcs(u8 stat)
{
    PCout(3) = stat;
}

void vs10xx_play_xdcs(u8 stat)
{
    PCout(2) = stat;
}

u8 vs10xx_play_wait_dq()
{
    while(vs10xx_play_dq() == 0);
	return 0;
}

void vs10xx_play_spi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB| RCC_APB2Periph_AFIO, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_SPI3, ENABLE );

    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable,ENABLE); //SWD

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    GPIO_ResetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);
    GPIO_SetBits(GPIOB,GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5);


    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI3, &SPI_InitStructure);

    SPI_Cmd(SPI3, ENABLE);

}


void vs10xx_play_dma_init(u32 cpar, u32 cmar)
{
	DMA_InitTypeDef DMA_InitStructure;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2, ENABLE);	//ʹ��DMA����
	
	DMA_DeInit(VS10XX_PLAY_DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  //DMA�������ַ
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  //DMA�ڴ����ַ
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  //���ݴ��䷽�򣬴��ڴ��ȡ���͵�����
	DMA_InitStructure.DMA_BufferSize = 0;  //DMAͨ����DMA����Ĵ�С
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  //�����ַ�Ĵ�������
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  //�ڴ��ַ�Ĵ�������
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;  //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte; //���ݿ��Ϊ8λ
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  //����������ģʽ
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium; //DMAͨ�� xӵ�������ȼ� 
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  //DMAͨ��xû������Ϊ�ڴ浽�ڴ洫��
	DMA_Init(VS10XX_PLAY_DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���
}

//����һ��DMA����
void vs10xx_play_dma_enable(u16 cndtr)
{ 
	DMA_Cmd(VS10XX_PLAY_DMA_CHx, DISABLE );  //�ر� ��ָʾ��ͨ��      
 	DMA_SetCurrDataCounter(VS10XX_PLAY_DMA_CHx, cndtr);//DMAͨ����DMA����Ĵ�С
 	DMA_Cmd(VS10XX_PLAY_DMA_CHx, ENABLE);  //ʹ�� ��ָʾ��ͨ�� 
}	

void vs10xx_play_init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);  

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;            
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_3;    //RST XDCS XCS
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    vs10xx_play_spi_init();
	vs10xx_play_dma_init((u32)&SPI3->DR, (u32)vs10xx_play_dma_buff);
}

void vs10xx_play_cfg_setup(vs10xx_cfg_t* cfg)
{
    cfg->VS_Init               = vs10xx_play_init;
    cfg->VS_SPI_ReadWriteByte  = SPI3_ReadWriteByte;
    cfg->VS_SPI_SpeedLow       = vs10xx_play_spi_speedlow;
    cfg->VS_SPI_SpeedHigh      = vs10xx_play_spi_speedhigh;
    cfg->VS_WAIT_DQ            = vs10xx_play_wait_dq;
    cfg->VS_DQ                 = vs10xx_play_dq;
    cfg->VS_RST                = vs10xx_play_rst;
    cfg->VS_XCS                = vs10xx_play_xcs;
    cfg->VS_XDCS               = vs10xx_play_xdcs;

    if(cfg->VS_Init)
        cfg->VS_Init();
	VS_Ram_Test(cfg);
	VS_Sine_Test(cfg);

}

