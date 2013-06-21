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


#ifndef SPI_H
#define SPI_H

#define SPI_DATA_LENGTH        (35)
#define SPI_CRC_DEFAULT        (0x5F)

#define SPI_CMD		0xF0
#define SPI_CMD_RW	0xF0
#define SPI_CHECK	0x00
#define SPI_CMD_CHECK	0x00

/* possible spi statuses */
enum spi_status {
	NO_MODULE_ON_USB       = 0xFF,    // SPI not working (HW error)
	SPI_DISABLED           = 0x00,    // SPI not working (disabled)
	SPI_CRCM_OK            = 0x3F,    // SPI not ready (full buffer, last CRCM ok)
	SPI_CRCM_ERR           = 0x3E,    // SPI not ready (full buffer, last CRCM error)
	COMMUNICATION_MODE     = 0x80,    // SPI ready (communication mode)
	PROGRAMMING_MODE       = 0x81,    // SPI ready (programming mode)
	DEBUG_MODE             = 0x82,    // SPI ready (debugging mode)
	SPI_SLOW_MODE          = 0x83,    // SPI is not working on the background - Slow mode
	SPI_USER_STOP          = 0x07,    // state after stopSPI();
	SPI_DATA_READY         = 0x40,    // data ready
	SPI_UNKNOWN	       = 0x01,	 // undefined status
};

unsigned char count_crc_tx(unsigned char *, int);
unsigned char check_crc_rx(unsigned char *, int , int );


#endif // SPI_H
