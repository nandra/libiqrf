/*
 * 
 * Copyright (C) 2010 Marek Belisko <marek.belisko@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */


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
unsigned char count_crc_tx(unsigned char *buff, int len)
{
        unsigned char crc_val;
        int i = 0;

        crc_val = SPI_CRC_DEFAULT;

        for (i = 0; i < len; i++)
                crc_val ^= buff[i];

        return crc_val;
}

/* count CRC for received buffer */
unsigned char check_crc_rx(unsigned char *buff, int PTYPE, int len)
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

