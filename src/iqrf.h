/*
 * Public libiqrf header file
 * Copyright (C) 2010 Marek Belisko <marek.belisko@gmail.com>
 * 		 2013 Jan Tusil <jenda.tusil@gmail.com>
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
 * Locks iqrf device
 * @param iqrf	device to lock
 *
 * Notes about locking:
 * During normal operations (like iqrf_raw_write()) this library locks
 * device for you. Use locking only together with *_unlocked functions.
 */
void iqrf_lock(iqrf_t *iqrf);

/**
 * Unlocks locked device
 * @param iqrf	device to unlock
 * See notes above
 */
void iqrf_unlock(iqrf_t *iqrf);



/**
 * Gets list of connected devices. Each entry describes where the device is connected.
 * @param list	where to store entries
 * @param num	how many entries in list
 * @return	number of stored entries
 */
int iqrf_get_device_list(usb_addr list[], int num);


/**
 * Gets number of connected devices
 * @return	device count
 */
int iqrf_get_device_count(void);


/**
 * Opens an IQRF device
 * @param addr	device to open
 * @return	opened device or NULL
 * It is not possible to open one device more than once.
 */

iqrf_t *iqrf_device_open(usb_addr *addr);

/**
 * Closes IQRF device
 * @param iqrf	device
 */
void iqrf_device_close(iqrf_t *iqrf);


/**
 * Easier way how to open a device
 * @param interactive   list devices (stdout) and let user (stdin) select one
 * @param list          if interactive == false, list devices only
 * @param which         if (interactive || list) == false, device to connect
 * @return      if (list == true || error) => NULL, IQRF device otherwise
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

/**
 * Resets device
 * @param iqrf	device
 * You probably won't need this
 */
void iqrf_reset_device(iqrf_t *iqrf);

/**
 * Are there any data?
 * @param iqrf	device
 * @return	number of bytes ready (or 0)
 */
int iqrf_spi_data_ready(iqrf_t *iqrf);


/**
 * Writes raw data to usb programmer
 * @param iqrf	device
 * @param buff	data to write
 * @param len	how many bytes to write
 * @return	number of bytes written 
 *
 * Depending on the first byte, data will be processed
 * by IQRF module or programmer. In most cases, last byte 
 * serves as CRC checksum.
 */
int iqrf_raw_write(iqrf_t *iqrf, const unsigned char *buff, int len);
int iqrf_raw_write_unlocked(iqrf_t *iqrf, const unsigned char *buff, int len);




/**
 * Reads max rx_len bytes from usb programmer
 * @param iqrf	device
 * @param buff	buffer to write data to
 * @param rx_len maximal number of bytes to read
 * @return	-1 on error, number of bytes (nb) read otherwise
 * Notes:
 * 	On error, buff stays unchanged.
 * 	On success, all buffer is overwritten.
 *	If rx_len > nb, it means last few bytes are dummy.
 */
int iqrf_raw_read(iqrf_t *iqrf, unsigned char *buff, int rx_len);
int iqrf_raw_read_unlocked(iqrf_t *iqrf, unsigned char *buff, int rx_len);


/**
 * Writes raw data to connected device and gets anwser.
 * @param iqrf		device
 * @param data_buff	RAW data to send	
 * @param tx_len	lenght of outgoing data
 * @param rx_len	lenght of incoming data
 * @param check_crc	check crc?
 */

int iqrf_write_read_data(iqrf_t *iqrf, unsigned char *data_buff, int tx_len, int rx_len, int check_crc);






/******************* Compat only symbols *******************/

#if 0
int iqrf_write_data(iqrf_t *iqrf, unsigned char *data_buff, int tx_len);
#endif
/* 
 * following functions are highly used only for iqrf ide
 * and makes no sense to use them in applications
 * thats the reason why aren't documented well ;)
 */
int iqrf_count_tx_crc(unsigned char *buff, int len);

#ifdef __cplusplus
}
#endif

#endif // IQRF_DEV_H
