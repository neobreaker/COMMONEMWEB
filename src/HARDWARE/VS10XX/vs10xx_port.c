#include "vs10xx_port.h"
#include "mysys.h"

//SPIx 读写一个字节
//TxData:要写入的字节
//返回值:读取到的字节
u8 SPI1_ReadWriteByte(u8 TxData)
{
    u8 retry=0;
    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET) //检查指定的SPI标志位设置与否:发送缓存空标志位
    {
        retry++;
        if(retry>200)return 0;
    }
    SPI_I2S_SendData(SPI1, TxData); //通过外设SPIx发送一个数据
    retry=0;

    while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET) //检查指定的SPI标志位设置与否:接受缓存非空标志位
    {
        retry++;
        if(retry>200)return 0;
    }
    return SPI_I2S_ReceiveData(SPI1); //返回通过SPIx最近接收的数据
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
    SPI1_SetSpeed(SPI_BaudRatePrescaler_32);//设置到低速模式
}

void vs10xx_spi_speedhigh(void)
{
    SPI1_SetSpeed(SPI_BaudRatePrescaler_8);//设置到高速模式
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

    RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA|RCC_APB2Periph_SPI1, ENABLE );//PORTB，SPI1时钟使能

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP; //PA5.6.7复用推挽
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_SetBits(GPIOA,GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7); //PA5.6.7上拉

    SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;  //设置SPI单向或者双向的数据模式:SPI设置为双线双向全双工
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;       //设置SPI工作模式:设置为主SPI
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;       //设置SPI的数据大小:SPI发送接收8位帧结构
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;     //选择了串行时钟的稳态:时钟悬空高
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;    //数据捕获于第二个时钟沿
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;       //NSS信号由硬件（NSS管脚）还是软件（使用SSI位）管理:内部NSS信号有SSI位控制
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;        //定义波特率预分频的值:波特率预分频值为16
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;  //指定数据传输从MSB位还是LSB位开始:数据传输从MSB位开始
    SPI_InitStructure.SPI_CRCPolynomial = 7;    //CRC值计算的多项式
    SPI_Init(SPI1, &SPI_InitStructure);  //根据SPI_InitStruct中指定的参数初始化外设SPIx寄存器

    SPI_Cmd(SPI1, ENABLE); //使能SPI外设

}

void vs10xx_init()
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC|RCC_APB2Periph_GPIOE|RCC_APB2Periph_GPIOF, ENABLE);  //使能PC,PE,PF端口时钟

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;               //PC13
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;        //输入
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;    //PE6
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //推挽输出
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6|GPIO_Pin_7;//PF6,PF7
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;         //推挽输出
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

