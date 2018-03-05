#include "vs10xx_port.h"
#include "mysys.h"

//SPIx ��дһ���ֽ�
//TxData:Ҫд����ֽ�
//����ֵ:��ȡ�����ֽ�
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //���ָ����SPI��־λ�������:���ͻ���ձ�־λ
    {
        retry++;
        if(retry>200)return 0;
    }
    SPI_I2S_SendData(SPI1, TxData); //ͨ������SPIx����һ������
    retry=0;

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //���ָ����SPI��־λ�������:���ܻ���ǿձ�־λ
    {
        retry++;
        if(retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI1); //����ͨ��SPIx������յ�����
}

void SPI1_SetSpeed(u8 SpeedSet)
{
    assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
    SPI1->CR1&=0XFFC7;
    SPI1->CR1|=SpeedSet;
    SPI_Cmd(SPI1,ENABLE);
}

void vs10xx_spi_speedlow(void)
{
    SPI1_SetSpeed(SPI_BaudRatePrescaler_32);//���õ�����ģʽ
}

void vs10xx_spi_speedhigh(void)
{
    SPI1_SetSpeed(SPI_BaudRatePrescaler_8);//���õ�����ģʽ
}

u8 vs10xx_dq()
{
    return PCin(13);
}

void vs10xx_rst(u8 stat)
{
    PEout(6) = stat;
}

void vs10xx_xcs(u8 stat)
{
    PFout(7) = stat;
}

void vs10xx_xdcs(u8 stat)
{
    PFout(6) = stat;
}

u8 vs10xx_wait_dq()
{
    while(vs10xx_dq() == 0);
	return 0;
}

void vs10xx_spi_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );//PORTB��SPI1ʱ��ʹ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //PA5.6.7��������
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7); //PA5.6.7����

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //����SPI�������˫�������ģʽ:SPI����Ϊ˫��˫��ȫ˫��
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //����SPI����ģʽ:����Ϊ��SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;       //����SPI�����ݴ�С:SPI���ͽ���8λ֡�ṹ
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;     //ѡ���˴���ʱ�ӵ���̬:ʱ�����ո�
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    //���ݲ����ڵڶ���ʱ����
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;       //NSS�ź���Ӳ����NSS�ܽţ����������ʹ��SSIλ������:�ڲ�NSS�ź���SSIλ����
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;        //���岨����Ԥ��Ƶ��ֵ:������Ԥ��ƵֵΪ16
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  //ָ�����ݴ����MSBλ����LSBλ��ʼ:���ݴ����MSBλ��ʼ
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRCֵ����Ķ���ʽ
    SPI_Init(SPI1, &SPI_InitStructure);  //����SPI_InitStruct��ָ���Ĳ�����ʼ������SPIx�Ĵ���

    SPI_Cmd(SPI1, ENABLE); //ʹ��SPI����

}

void vs10xx_init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF, ENABLE);  //ʹ��PC,PE,PF�˿�ʱ��

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;               //PC13
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        //����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;    //PE6
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //�������
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;//PF6,PF7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //�������
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    vs10xx_spi_init();
}

void vs10xx_cfg_setup(vs10xx_cfg_t* cfg)
{
    cfg->VS_Init                = vs10xx_init;
    cfg->VS_SPI_ReadWriteByte   = SPI1_ReadWriteByte;
    cfg->VS_SPI_SpeedLow        = vs10xx_spi_speedlow;
    cfg->VS_SPI_SpeedHigh       = vs10xx_spi_speedhigh;
    cfg->VS_WAIT_DQ         	= vs10xx_wait_dq;
    cfg->VS_DQ                  = vs10xx_dq;
    cfg->VS_RST             	= vs10xx_rst;
    cfg->VS_XCS             	= vs10xx_xcs;
    cfg->VS_XDCS                = vs10xx_xdcs;

    if(cfg->VS_Init)
        cfg->VS_Init();
    VS_Ram_Test(cfg);
}

