#ifndef __sht1x_included__
#define __sht1x_included__

#define SHT_SCK_BIT     (1<<PB3)
#define SHT_SCK_PORT    PORTB
#define SHT_SCK_DDR     DDRB

#define SHT_DATA_BIT    (1<<PB2)
#define SHT_DATA_PORT   PORTB
#define SHT_DATA_DDR    DDRB
#define SHT_DATA_PIN    PINB

#define SHT_CMD_MEASURE_TEMPERATURE 0x03
#define SHT_CMD_MEASURE_HUMIDITY    0x05
#define SHT_CMD_READ_STATUS         0x07
#define SHT_CMD_WRITE_STATUS        0x06
#define SHT_CMD_SOFT_RESET          0x36

void sht1x_init(void);
int  sht1x_measure(unsigned char cmd, unsigned char dst[2]);

#endif // __sht1x_included__

