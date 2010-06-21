/*
 * Public libiqrf header file
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

#ifndef IQRF_DEV_H
#define IQRF_DEV_H

#ifdef __cplusplus
extern "C" {
#endif

/* maximum length for SPI data */
#define SPI_DATA_LENGTH (35)
/* command definition for CK */
#define CMD_FOR_CK 0x01

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

/* return status of SPI communication */
enum spi_status iqrf_get_spi_status(void);

/* 
 * this function have 2 functionalities
 * for data reading you provide a buffer 
 * data_buff where data will be written
 * after reading
 *
 * for writing you put your data in data_buff
 * and then response will be also placed
 * in data_buff (strange but TRUE :))
 *
 */
int iqrf_read_write_spi_cmd_data(unsigned char *data_buff, int data_len, int read_write);

/* device initialization */
int iqrf_init_device();

/* device release */
void iqrf_release_device(void);

/* reset device (also usb reset)*/
void iqrf_reset_device();

/* 
 * following functions are highly used only for iqrf ide
 * and makes no sense to use them in applications
 * thats the reason why aren't documented well ;)
 */
int iqrf_write_read_data(unsigned char *data_buff, int tx_len, int rx_len, int check_crc);
int iqrf_write_data(unsigned char *data_buff, int tx_len);
int iqrf_count_tx_crc(unsigned char *buff, int len);

#ifdef __cplusplus
}
#endif

#endif // IQRF_DEV_H
