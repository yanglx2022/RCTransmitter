/**********************
* 2.4G无线  BK2425
**********************/
#include "bk2425.h"
#include "print.h"

#define ADDR_WIDTH   			3  		// 地址宽度(3~5bytes)
const static uint8_t ADDRESS[ADDR_WIDTH] = {0x4A, 0x22, 0x50};// 地址

// NRF24L01操作线
#define CE(n)		gpio_bit_write(GPIOB, GPIO_PIN_7, n ? SET : RESET)	// 片选
#define CSN(n)	gpio_bit_write(GPIOB, GPIO_PIN_6, n ? SET : RESET)	// SPI片选
#define MOSI(n)	gpio_bit_write(GPIOB, GPIO_PIN_4, n ? SET : RESET)	// MOSI
#define SCK(n)	gpio_bit_write(GPIOB, GPIO_PIN_5, n ? SET : RESET)	// SCK
#define MISO		gpio_input_bit_get(GPIOB, GPIO_PIN_3)								// 读MISO

// 是否初始化成功
static uint8_t initialized = 0;

static uint8_t Write_Reg(uint8_t reg, uint8_t value);
static uint8_t BK2425_Check(void);
static void BK2425_TX_Mode(void);

// 初始化
uint8_t BK2425_Init(void)
{
	/** GPIO 配置
  PB6 ------> CSN
	PB7 ------> CE
	PB4 ------> MOSI
	PB5 ------> SCK
	PB3 ------> MISO */
	rcu_periph_clock_enable(RCU_GPIOB);
	gpio_mode_set(GPIOB, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_4 | GPIO_PIN_5);
	gpio_output_options_set(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_4 | GPIO_PIN_5);
	gpio_mode_set(GPIOB, GPIO_MODE_INPUT, GPIO_PUPD_PULLUP, GPIO_PIN_3);
	// 清TX FIFO(防止复位时发送复位前最后一次的数据)
	Write_Reg(FLUSH_TX, 0xFF);
	// SPI
	SCK(0);
	CSN(1);
	CE(0);
	// check
	initialized = BK2425_Check();
	if (initialized)
	{
		// TX Mode
		BK2425_TX_Mode();
	}
	return initialized;
}

// 查询初始化是否成功
uint8_t BK2425_IsReady(void)
{
	return initialized;
}

// 模拟SPI读写字节
static uint8_t SPI_RW(uint8_t byte)
{
	for(uint8_t i = 0; i < 8; i++)
	{
		MOSI(byte & 0x80);														 
		byte = byte << 1;                      
		SCK(1);                                   
		byte |= MISO;                             
		SCK(0);                                
	}
	return(byte);                              
}

// 写寄存器(单字节)
static uint8_t Write_Reg(uint8_t reg, uint8_t value)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	SPI_RW(value);
	CSN(1);
	return status;
}

// 写寄存器(多字节)
static uint8_t Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t bytes)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	for(uint8_t i = 0; i < bytes; i++)
	{
		SPI_RW(*pBuf++);
	}
	CSN(1);
	return status;
}		

// 读寄存器(单字节)
static uint8_t Read_Reg(uint8_t reg)
{
	CSN(0);
	SPI_RW(reg);
	uint8_t val = SPI_RW(0);
	CSN(1);
	return val;
}

// 读寄存器(多字节)
static uint8_t Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes)
{
	CSN(0);
	uint8_t status = SPI_RW(reg);
	for(uint8_t i = 0; i < bytes; i++)
	{
		pBuf[i] = SPI_RW(0);
	}
	CSN(1);
	return status;
}

// 检测BK2425
static uint8_t BK2425_Check(void)
{
	uint8_t buf[5] = {0xA5, 0xA5, 0xA5, 0xA5, 0xA5};
	Write_Buf(W_REGISTER + TX_ADDR, buf, 5);
	Read_Buf(TX_ADDR, buf, 5);
	for(uint8_t i = 0; i < 5; i++)
	{
		if (buf[i] != 0xA5)
		{
			return 0;	// 检测错误
		}
	}
	return 1;
}

// 发送模式初始化
static void BK2425_TX_Mode(void)
{
	CE(0);
	Write_Reg(W_REGISTER + CONFIG, 0x7E);      							// 关闭所有中断,CRC使能2byte,开机,发送模式
	Write_Reg(W_REGISTER + EN_AA, 0x01);        						// 使能通道0自动应答
	Write_Reg(W_REGISTER + EN_RXADDR, 0x01);        				// 使能接收管道0
	Write_Reg(W_REGISTER + SETUP_AW, ADDR_WIDTH - 2);       // 地址宽度
	Write_Reg(W_REGISTER + SETUP_RETR, 0x15);  							// 自动重发延时500us最大5次
	Write_Reg(W_REGISTER + RF_CH, 20);                 			// 选择射频信道2420MHz
	Write_Reg(W_REGISTER + RF_SETUP, 0x27);            			// 空中波特率250Kbps,发射功率最大
	Write_Reg(W_REGISTER + STATUS, 0xFF);  									// 清除所有的标志位
	Write_Buf(W_REGISTER + TX_ADDR, ADDRESS, ADDR_WIDTH);		// 发送地址
	Write_Buf(W_REGISTER + RX_ADDR_P0, ADDRESS, ADDR_WIDTH);// 接收通道0地址和发送地址相同
//	CE(1);
}

// 发送数据
uint8_t BK2425_Transmit(uint8_t* payload)
{
	CE(0);
	Write_Buf(WR_TX_PLOAD, payload, PAYLOAD_WIDTH);
	CE(1);
	while(1)
	{
		uint8_t status = Read_Reg(STATUS);
		if (status & TX_DS)				// 发送完成
		{
			Write_Reg(W_REGISTER + STATUS, 0xFF);
			//printf("%x\t%d\t%d\t%d\t%d\t\r\n", payload[1], *(int16_t*)&payload[2], *(int16_t*)&payload[4], *(int16_t*)&payload[6] ,*(int16_t*)&payload[8]);
			return 1;
		}
		else if (status & MAX_RT)	// 达到最大重发次数
		{
			Write_Reg(W_REGISTER + STATUS, 0xFF);
			Write_Reg(FLUSH_TX, 0xFF);
			return 0;
		}
	}
}

