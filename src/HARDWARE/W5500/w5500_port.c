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
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);  //ʹ��DMA����

    DMA_DeInit(W5500_DMA_CHx);   //��DMA��ͨ��1�Ĵ�������Ϊȱʡֵ

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

    DMA_Init(W5500_DMA_CHx, &DMA_InitStructure);  //����DMA_InitStruct��ָ���Ĳ�����ʼ��DMA��ͨ��USART1_Tx_DMA_Channel����ʶ�ļĴ���

    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
    NVIC_InitStructure.NVIC_IRQChannel = DMA1_Channel5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    DMA_ITConfig(W5500_DMA_CHx, DMA_IT_TC, ENABLE);

}

//����һ��DMA����
void w5500_dma_enable(u16 cndtr)
{
    u8 err;
    SPI_I2S_DMACmd(SPI2, SPI_I2S_DMAReq_Tx, ENABLE);
    DMA_Cmd(W5500_DMA_CHx, DISABLE );               //�ر� ��ָʾ��ͨ��
    DMA_SetCurrDataCounter(W5500_DMA_CHx, cndtr);   //DMAͨ����DMA����Ĵ�С
    DMA_Cmd(W5500_DMA_CHx, ENABLE);                 //ʹ�� ��ָʾ��ͨ��
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

    /* ��ʼ��SCK��MISO��MOSI���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);

    /* ��ʼ��CS���� */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode=GPIO_Mode_Out_PP;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    GPIO_SetBits(GPIOB, GPIO_Pin_12);

    /* ��ʼ������STM32 SPI2 */
    SPI_InitStructure.SPI_Direction=SPI_Direction_2Lines_FullDuplex;    //SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode=SPI_Mode_Master;                         //����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize=SPI_DataSize_8b;                     //SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL=SPI_CPOL_Low;                            //ʱ�����յ�
    SPI_InitStructure.SPI_CPHA=SPI_CPHA_1Edge;                          //���ݲ����ڵ�1��ʱ����
    SPI_InitStructure.SPI_NSS=SPI_NSS_Soft;                             //NSS���ⲿ�ܽŹ���
    SPI_InitStructure.SPI_BaudRatePrescaler=SPI_BaudRatePrescaler_2;    //������Ԥ��ƵֵΪ2
    SPI_InitStructure.SPI_FirstBit=SPI_FirstBit_MSB;                    //���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial=7;                              //CRC����ʽΪ7
    SPI_Init(SPI2,&SPI_InitStructure);                                  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPI1�Ĵ���

    SPI_Cmd(SPI2,ENABLE);   //STM32ʹ��SPI1
}

void W5500_GPIO_Configuration(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOG , ENABLE);
    /* W5500_RST���ų�ʼ������*/
    GPIO_InitStructure.GPIO_Pin  = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Speed=GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOG, &GPIO_InitStructure);
    GPIO_ResetBits(GPIOG, GPIO_Pin_6);

    SPI_Configuration();
    w5500_dma_init((u32)&SPI2->DR, (u32)w5500_send_buffer);
}

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI2_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
    {
        retry++;
        if(retry>200)return 0;
    }
    SPI_I2S_SendData(SPI2, TxData); //ͨ������SPIx����һ������
    retry=0;

    while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
    {
        retry++;
        if(retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI2); //����ͨ��SPIx������յ�����
}

void SPI2_WriteShort(unsigned short dat)
{
    u8 *p = (u8*)&dat;
    SPI2_ReadWriteByte(*(p+1));     //д���ݸ�λ
    SPI2_ReadWriteByte(*p);         //д���ݵ�λ
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

