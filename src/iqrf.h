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

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

	
/* maximum length for SPI data */
#define SPI_DATA_LENGTH (35)
/* command definition for CK */
#define CMD_FOR_CK 0x01

/* Data types */

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

typedef struct {
	uint8_t bus;
	uint8_t port;
} usb_addr;

struct iqrf_struct;
typedef struct iqrf_struct iqrf_t;

/* Functions */

/* return status of SPI communication */
enum spi_status iqrf_get_spi_status(iqrf_t *iqrf);


/**
 * Initialize IQRF library.
 * @return	0 on success
 */
int iqrf_init(void);

/**
 * Deinitialize IQRF library.
 */
void iqrf_exit(void);


/**
 * Gets list of connected devices. Each entry describes where the device is connected.
 * @param list	where to store entries
 * @param num	how many entries in list
 * @return	number of stored entries
 */
int iqrf_get_device_list(usb_addr list[], int num);


int iqrf_get_device_count(void);


/**
 * Opens an IQRF device
 * @param addr	device to open
 * @return	opened device or NULL
 */

iqrf_t *iqrf_device_open(usb_addr *addr);

/**
 * Closes IQRF device
 * @param iqrf	device
 */
void iqrf_device_close(iqrf_t *iqrf);


/**
 * Easier way how to open a device
 * @param interactive   list devices and let user select one
 * @param list          if interactive == false, list devices only
 * @param which         if (interactive || list) == false, device to connect
 * @return      if list == true || error => NULL, IQRF device otherwise
 */

iqrf_t *iqrf_select_device(bool interactive, bool list, int which);


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

int iqrf_read_write_spi_cmd_data(iqrf_t *iqrf, unsigned char *data_buff, int data_len, int read_write);

/* reset device (also usb reset)*/
void iqrf_reset_device(iqrf_t *iqrf);

int iqrf_spi_data_ready(iqrf_t *iqrf);


/* 
 * following functions are highly used only for iqrf ide
 * and makes no sense to use them in applications
 * thats the reason why aren't documented well ;)
 */
int iqrf_write_read_data(iqrf_t *iqrf, unsigned char *data_buff, int tx_len, int rx_len, int check_crc);
int iqrf_write_data(iqrf_t *iqrf, unsigned char *data_buff, int tx_len);
int iqrf_count_tx_crc(unsigned char *buff, int len);

#ifdef __cplusplus
}
#endif

#endif // IQRF_DEV_H
