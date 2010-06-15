#include <stdio.h>
#include "spi.h"

//#define DEBUG_SPI

#ifdef DEBUG_SPI
#define DBG(fmt, args...) \
printf("spi:" fmt "\n", ##args);
#else
#define DBG(fmt, args...) {}
#endif


/* count CRC for tx buffer*/
unsigned char iqrf_spi::count_crc_tx(unsigned char *buff, int len)
{
        unsigned char crc_val;
        int i = 0;

        crc_val = SPI_CRC_DEFAULT;

        for (i = 0; i < len; i++)
                crc_val ^= buff[i];

        return crc_val;
}

/* count CRC for received buffer */
unsigned char iqrf_spi::check_crc_rx(unsigned char *buff, int PTYPE, int len)
{
        unsigned char i, crc_val;

        crc_val = SPI_CRC_DEFAULT ^ PTYPE;

        for (i = 0; i < len; i++) {
                DBG("crc = %x, buff[%d]=%x", crc_val, i, buff[i]);
                crc_val ^= buff[i];
        }

        if (buff[len] == crc_val)
                return 1;

        DBG("Wrong checksum!");
        return 0;
}

