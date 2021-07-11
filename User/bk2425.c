/**********************
* 2.4G无线  BK2425
**********************/
#include "bk2425.h"
#include "print.h"
#include "fhss.h"

// 地址宽度4字节(3~5bytes)
static const uint8_t ADDR_WIDTH = 4;

// NRF24L01操作线
#define CE(n)		gpio_bit_write(GPIOB, GPIO_PIN_7, n ? SET : RESET)	// 片选
#define CSN(n)	gpio_bit_write(GPIOB, GPIO_PIN_6, n ? SET : RESET)	// SPI片选
#define MOSI(n)	gpio_bit_write(GPIOB, GPIO_PIN_4, n ? SET : RESET)	// MOSI
#define SCK(n)	gpio_bit_write(GPIOB, GPIO_PIN_5, n ? SET : RESET)	// SCK
#define MISO		gpio_input_bit_get(GPIOB, GPIO_PIN_3)								// 读MISO

static uint8_t Write_Reg(uint8_t reg, uint8_t value);
static uint8_t Write_Buf(uint8_t reg, const uint8_t *pBuf, uint8_t bytes);
static uint8_t BK2425_Check(void);
static uint8_t Read_Reg(uint8_t reg);
static uint8_t Read_Buf(uint8_t reg, uint8_t *pBuf, uint8_t bytes);

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
	// 清FIFO(防止复位时发送复位前最后一次的数据)
	Write_Reg(FLUSH_TX, 0xFF);
	Write_Reg(FLUSH_RX, 0xFF);
	// SPI
	SCK(0);
	CSN(1);
	CE(0);
	// check
	uint8_t check = BK2425_Check();
	return check;
}

// 设置频道
void BK2425_RF_CH(uint8_t rf_ch)
{
	CE(0);
	Write_Reg(W_REGISTER + RF_CH, rf_ch);	// 修改频道
	CE(1);
}

// 发送模式初始化
void BK2425_TX_Mode(uint32_t address)
{
	CE(0);
	Write_Reg(W_REGISTER + CONFIG, 0x7E);      													// 关闭所有中断,CRC使能2byte,开机,发送模式
	Write_Reg(W_REGISTER + EN_AA, 0x01);        												// 使能通道0自动应答
	Write_Reg(W_REGISTER + EN_RXADDR, 0x01);        										// 使能接收管道0
	Write_Reg(W_REGISTER + SETUP_AW, ADDR_WIDTH - 2);       						// 地址宽度
	Write_Reg(W_REGISTER + SETUP_RETR, 0x25);  													// 自动重发延时750us最大5次
	Write_Reg(W_REGISTER + RF_CH, RF_CH_DEFAULT);           						// 选择射频信道
	Write_Reg(W_REGISTER + RF_SETUP, 0x27);            									// 空中波特率250Kbps,发射功率最大
	Write_Reg(W_REGISTER + STATUS, 0xFF);  															// 清除所有的标志位
	Write_Buf(W_REGISTER + TX_ADDR, (uint8_t*)&address, ADDR_WIDTH);		// 发送地址
	Write_Buf(W_REGISTER + RX_ADDR_P0, (uint8_t*)&address, ADDR_WIDTH);	// 接收通道0地址和发送地址相同
}

// 发送数据
uint8_t BK2425_Transmit(uint8_t* data, uint8_t length)
{
	// 发送
	CE(0);
	Write_Buf(WR_TX_PLOAD, data, length);
	CE(1);
	// 阻塞查询
	uint8_t status;
	while(1)
	{
		status = Read_Reg(STATUS);
		if (status & TX_DS)				// 发送成功
		{
			Write_Reg(W_REGISTER + STATUS, 0xFF);	// 清中断标志
			return 1;
		}
		else if (status & MAX_RT)	// 达到最大重发次数
		{
			Write_Reg(FLUSH_TX, 0xFF);						// 清TX FIFO
			Write_Reg(W_REGISTER + STATUS, 0xFF);	// 清中断标志
			return 0;
		}
	}
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


