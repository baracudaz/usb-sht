#include <avr/io.h>
#include "sht1x.h"

static void sht1x_delay()
{
	// 1 us (at 1MHz clock ??)
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
	asm volatile ("nop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop\nnop");
}

void sht1x_init(void)
{
	/* Clock output, normally low */
	SHT_SCK_DDR  |=  SHT_SCK_BIT;
	SHT_SCK_PORT &= ~SHT_SCK_BIT;

	/* Clock as input, with pullup */
	SHT_DATA_DDR  &= ~SHT_DATA_BIT;
	SHT_DATA_PORT |=  SHT_DATA_BIT;
}

static void sht1x_pull_data()
{
	/* Disable pullup first in order to pull
	 * when we change the direction to output */
	SHT_DATA_PORT &= ~SHT_DATA_BIT;
	SHT_DATA_DDR  |=  SHT_DATA_BIT;
}

static void sht1x_release_data()
{
	SHT_DATA_DDR  &= ~SHT_DATA_BIT;
	SHT_DATA_PORT |=  SHT_DATA_BIT;
}

static void sht1x_clock_high()
{
	SHT_SCK_PORT |= SHT_SCK_BIT;
}

static void sht1x_clock_low()
{
	SHT_SCK_PORT &= ~SHT_SCK_BIT;
}

int sht1x_cmd(unsigned char cmd)
{
	char i;
	char ack;

	/* Transmission Start */
	sht1x_clock_high(); 
	sht1x_delay();
	sht1x_pull_data(); 
	sht1x_delay();
	sht1x_clock_low();
	sht1x_delay();
	sht1x_clock_high();	
	sht1x_delay();
	sht1x_release_data();
	sht1x_delay();
	sht1x_clock_low();
	sht1x_delay();

	/* 3 address bits + 5 command bits */
	for (i=0; i<8; i++)
	{
		if (cmd & 0x80)
			sht1x_release_data();
		else
			sht1x_pull_data();
		
		sht1x_delay();
		sht1x_clock_high();
		sht1x_delay();
		sht1x_clock_low();

		cmd <<= 1;
	}
	sht1x_release_data();

	/* ack */	
	sht1x_delay();
	sht1x_clock_high();
	sht1x_delay();
	ack = SHT_DATA_PIN & SHT_DATA_BIT;
	sht1x_clock_low();
	
	if (ack) {
		return -1; // no ack!
	}

	// let the slave relase data
	while (!(SHT_DATA_PIN & SHT_DATA_BIT)) 
		{ /* empty */	}

	return 0;
}

/** \brief Read and ack a byte from the sensor
 *
 * Note: This should be called after the transmission
 * start, address and commands are sent and after the slave
 * has pulled data low again indicating that the conversion
 * is completed. */
int sht1x_read_byte(unsigned char *dst, char skip_ack)
{
	unsigned char tmp;
	int i;

	for (tmp=0,i=0; i<8; i++) {
		sht1x_delay();
		sht1x_clock_high();
		sht1x_delay();
		tmp <<= 1;
		if (SHT_DATA_PIN & SHT_DATA_BIT) {
			tmp |= 1;
		} else {
			// tmp &= ~1;
		}
		sht1x_clock_low();
	}
	*dst = tmp;
	
	/* Ack the byte by pulling data low during a 9th clock cycle */
	if (!skip_ack)
		sht1x_pull_data();
	sht1x_delay();
	sht1x_clock_high();
	sht1x_delay();
	sht1x_clock_low();
	sht1x_release_data();
	sht1x_delay();

	return 0;
}

int sht1x_measure(unsigned char cmd, unsigned char dst[2])
{
	if (sht1x_cmd(cmd))
		return -1;
	
	/* The slave pulls data low when conversion is done */
	while ((SHT_DATA_PIN & SHT_DATA_BIT)) 
		{ /* empty */	}

	sht1x_read_byte(&dst[0], 0);
	sht1x_read_byte(&dst[1], 1);
//	sht1x_read_byte(&crc, 1);

	return 0;
}




