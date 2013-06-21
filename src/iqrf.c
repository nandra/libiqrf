/*
 * libiqrf interface API
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
 *
 * TODO: Use crc checks
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <semaphore.h>
#include <stdbool.h>

#include "iqrf-dev.h"
#include "spi.h"
#include "usb.h"


#define CMD_FOR_CK 0x01

/* debug */
#define DEBUG_PREPEND "[debug] iqrf: "
#include "debug.h"

#define ERR_LOCK(x) ERR("can't get semaphore in '%s'\n", "dummy")

int iqrf_init(void)
{
	return usb_init();
}

void iqrf_exit(void)
{
	usb_exit();
}

void iqrf_lock(iqrf_t *iqrf)
{
	if (sem_wait(&iqrf->sem))
		ERR_LOCK();
}

void iqrf_unlock(iqrf_t *iqrf)
{
	sem_post(&iqrf->sem);
}

int iqrf_get_device_list(usb_addr list[], int n_devices)
{
	int n;
	n = usb_get_device_list(list, n_devices, false);
	if (n < 0)
		ERR("can't get device list\n");
	return n;
}

int iqrf_get_device_count(void)
{
	int n;
	n = usb_get_device_list(NULL, 256, true);
	if (n < 0)
		ERR("can't get device count\n");
	return n;
}

iqrf_t *iqrf_device_open(usb_addr *addr)
{
	iqrf_t *iqrf;
	int rv;

	iqrf = malloc(sizeof(*iqrf));
	if (!iqrf)
		goto err_mem;
	rv = sem_init(&iqrf->sem, 0, 1);
	if (rv)
		goto err_sem;
	iqrf->dev = usb_device_open_by_addr(addr);
	if (!iqrf->dev)
		goto err_dev;
	DBG("device opened\n");
	return iqrf;

err_dev:
	sem_destroy(&iqrf->sem);
err_sem:
	free(iqrf);
err_mem:
	DBG("error while opening device\n");
	return NULL;
}

void iqrf_device_close(iqrf_t *iqrf)
{
	/* get semaphore */
	iqrf_lock(iqrf);

	usb_device_close(iqrf->dev);
	iqrf->dev = NULL;

	/* see SEM_DESTROY(3) (destroying blocked semaphore) */
	sem_destroy(&iqrf->sem);

	free(iqrf);
	DBG("device closed\n");
	return;
}


int iqrf_raw_write_unlocked(iqrf_t *iqrf, const unsigned char *buff, int tx_len)
{
	int rv;

    	usb_set_tx_len(iqrf->dev, tx_len);
    	usb_write_tx_buff(iqrf->dev, buff, tx_len);
    	rv = usb_send_packet(iqrf->dev);

	return rv;
}

int iqrf_raw_write(iqrf_t *iqrf, const unsigned char *data_buff, int tx_len)
{
	int rv;
	iqrf_lock(iqrf);
	rv = iqrf_raw_write_unlocked(iqrf, data_buff, tx_len);
   	iqrf_unlock(iqrf);
    	return rv;
}


int iqrf_raw_read_unlocked(iqrf_t *iqrf, unsigned char *buff, int rx_len)
{
	int rv;
	usb_set_rx_len(iqrf->dev, rx_len);
	rv = usb_retrieve_packet(iqrf->dev);
	if (rv < 0) {
		perror("retrieve");
		return -1;
	}
	/* it returns number of bytes copied */
	rv = usb_read_rx_buff(iqrf->dev, buff);
	return rv;
}

int iqrf_raw_read(iqrf_t *iqrf, unsigned char *buff, int rx_len)
{
	int rv;
	iqrf_lock(iqrf);
	rv = iqrf_raw_read_unlocked(iqrf, buff, rx_len);
	iqrf_unlock(iqrf);
	return rv;
}

#define SPI_MAX_LEN	41	// for OS 3.0
//#define SPI_MAX_LEN	64	// for OS 3.01D

/* get spi_status of module spi */
int iqrf_get_spi_status(iqrf_t *iqrf)
{
    	unsigned char buff[BUF_LEN];
//    	int len = 0;
	enum spi_status status;

	iqrf_lock(iqrf);

	buff[0] = CMD_FOR_CK;
    	buff[1] = 0;
    	buff[2] = 0;
    	buff[3] = 0;
    	buff[4] = 0;
    	usb_set_tx_len(iqrf->dev, 5);
    	usb_set_rx_len(iqrf->dev, 4);

	/* TODO: one write, one read */
	usb_tx_buff_write(iqrf->dev, buff, 5);
	usb_send_receive_packet(iqrf->dev);
	usb_read_rx_buff(iqrf->dev, buff);

	iqrf_unlock(iqrf);

	switch(buff[1]) {
    	case SPI_DISABLED:
    	case SPI_USER_STOP:
    	case SPI_CRCM_OK:
    	case SPI_CRCM_ERR:
    	case COMMUNICATION_MODE:
    	case PROGRAMMING_MODE:
    	case DEBUG_MODE:
    	case SPI_SLOW_MODE:
    	case NO_MODULE_ON_USB:
    		status = buff[1];
	 	break;
    	default:
         	if (buff[1] >= 0x40 && buff[1] <= 0x40 + SPI_MAX_LEN)
              		status = buff[1];
         	else {
			fprintf(stderr, "unknown spi state: %d\n", buff[1]);
		 	status = SPI_UNKNOWN;
		}

         	break;
    	}

    	DBG("%lu:spi_status:%X\n", time(&tm), buff[1]);
    	return status;
}

/**
 * Returns how many data bytes are waiting for us
 */
int iqrf_spi_data_ready(iqrf_t *iqrf)
{
	int rv;

	rv = iqrf_get_spi_status(iqrf);


	if (rv < 0x40)
		return 0;
	if (rv > 0x40 + SPI_MAX_LEN)
		return 0;

	if (rv == 0x40)
		return 64;

	return rv - 0x40;
}


#define IQRF_F_WRITE	0x01
#define IQRF_F_READ	0x02
#define IQRF_F_CRC	0x04
#define IQRF_F_LOCK	0x08
/**
 * Performs I/O operation on IQRF module
 * @param iqrf	module
 * @param din	buffer where to store input data
 * @param dout	data to send
 * @param len	how many bytes to read or write
 * @int flags	what to do
 *
 * @return	-1 on error, number of bytes sent/recved otherwise
 */


int iqrf_module_perform(iqrf_t *iqrf, unsigned char *din, const unsigned char *dout, int len, int flags)
{
	unsigned char buff[BUF_LEN];
	unsigned char ptype;
	unsigned char crc_out;

	int rv;
	int bytes;

	if (len > SPI_MAX_LEN || len < 0)
		return -1;
	if ((flags & (IQRF_F_READ | IQRF_F_WRITE)) == 0)
		return -1;


	/* write? */
	ptype = len;
	if (flags & IQRF_F_WRITE)
		ptype |= 0x80;

	memset(buff, 0, sizeof(buff));
	buff[0] = CMD_FOR_CK;	// for programmer
	buff[1] = SPI_CMD_RW;	// for device - data read/write
	buff[2] = ptype;
	if ((flags & IQRF_F_WRITE) && dout != NULL) {
		memcpy(buff+3, dout, len);
	}
	 crc_out = count_crc_tx(buff+1, len+3);
	 buff[len+3] = crc_out;
	

	/* lock */
	if (flags & IQRF_F_LOCK)
		iqrf_lock(iqrf);

	rv = iqrf_raw_write_unlocked(iqrf, buff, len + 4);
	/* some error? */
	if (rv < 0)
		goto error;

	/* ok, maybe it is possible that we have written only 'rv' bytes
	 * and module's answer will have the same lenght */
	bytes = rv;

	rv = iqrf_raw_read_unlocked(iqrf, buff, bytes);
	if (rv < 0)
		goto error;

	/* Uch, this is strange. */
	if (rv != bytes)
		goto error;

	/* BTW we can get status here */
//	status = buff[0];

	/* check CRC */
	if (flags & IQRF_F_CRC) {
		if (check_crc_rx(buff + 2, ptype, bytes) == 0)
			goto error;

	}

	if (flags & IQRF_F_LOCK)
		iqrf_unlock(iqrf);

	/* read it ? */
	if ((flags & IQRF_F_READ) && din != NULL) {
		memcpy(din, buff + 2, bytes);
	}

	return bytes;

error:
	if (flags & IQRF_F_LOCK)
		iqrf_unlock(iqrf);
	return -1;
}


int iqrf_module_write_unlocked(iqrf_t *iqrf, const unsigned char *data, int len)
{
	int f;
	f = IQRF_F_WRITE;
	return iqrf_module_perform(iqrf, NULL, data, len, f);
}

int iqrf_module_write(iqrf_t *iqrf, const unsigned char *data, int len)
{
	int f;
	f = IQRF_F_WRITE | IQRF_F_LOCK;
	return iqrf_module_perform(iqrf, NULL, data, len, f);
}

int iqrf_module_read_unlocked(iqrf_t *iqrf, unsigned char *data, int len)
{
	int f;
	f = IQRF_F_READ;
	return iqrf_module_perform(iqrf, data, NULL, len, f);
}

int iqrf_module_read(iqrf_t *iqrf, unsigned char *data, int len)
{
	int f;
	f = IQRF_F_READ | IQRF_F_LOCK;
	return iqrf_module_perform(iqrf, data, NULL, len, f);
}

#if 0
/**
 * Sends user data to IQRF module
 */
int iqrf_read_write_spi_cmd_data(iqrf_t *iqrf, unsigned char *data_buff, int data_len, int read_write)
{
	unsigned char buff[BUF_LEN], PTYPE = 0;
    	int i, len, crc_rx, ret_val = 0;

	iqrf_lock(iqrf);
    	/* avoid get longer data line 35 bytes */
    	data_len &= 0x3F;

    	memset(buff, 0, sizeof(buff));

	/* check if master can read write or read only */
    	if (read_write)
        	PTYPE = data_len | 0x80;
    	else
        	PTYPE = data_len & 0x7F;
	/* create header*/
	buff[0] = CMD_FOR_CK;	// send it to IQRF module
    	buff[1] = SPI_CMD_RW;	// 0xF0 - data read/write
    	buff[2] = PTYPE;	// r/w, len

	/* copy user data */
	memcpy(&buff[3], data_buff, data_len);

	/* calculate checksum */
    	buff[data_len + 3] = count_crc_tx(&buff[1], data_len + 3);

    	usb_set_tx_len(iqrf->dev, data_len + 4);
    	usb_set_rx_len(iqrf->dev, data_len + 4);
#if 0
	/* dump data */
	for (i = 0; i < data_len + 4; i++)
        	DBG("data transfered[%d]:0x%X\n", i, buff[i]);
    	DBG("\n");
#endif

	/* send it */
	usb_write_tx_buff(iqrf->dev, buff, data_len + 4);

	/* TODO: use one read and one write */
	usb_send_receive_packet(iqrf->dev);

	/* retrieve data */
	len = usb_read_rx_buff(iqrf->dev, buff);
    	/* count crc for retrieved data */
	crc_rx = check_crc_rx(&buff[2], PTYPE, data_len);

	if (crc_rx) {
		if (read_write) {
			/* whole buff without crc */
			memcpy(data_buff, &buff[0], len - 1);
        		ret_val = len;

		} else {
			// TEST: remove memcpy()
        		memcpy(data_buff, &buff[2], data_len);
        		ret_val = data_len;
		}
    	} else {
        	/* this could occur in case of module info */
        	memcpy(data_buff, &buff[2], 4);
//		perror("checksum\n");
        	DBG("Wrong data checksum\n");
    	}
    	sem_post(&iqrf->sem);
    	return ret_val;

}
#endif


/* Programmer does some magic there */

int iqrf_write_read_data(iqrf_t *iqrf, unsigned char *data_buff, int tx_len, int rx_len, bool check_crc)
{
//	unsigned char PTYPE;
    	int len;

	/* we don't use it */
	(void)check_crc;

	/* get semaphore */
	if (sem_wait(&iqrf->sem))
		ERR_LOCK();

//	/* get ptype (will be needed during CRC check) */
  //  	PTYPE = data_buff[2];

	usb_set_tx_len(iqrf->dev, tx_len);
    	usb_set_rx_len(iqrf->dev, rx_len);

    	usb_write_tx_buff(iqrf->dev, data_buff, tx_len);
	/* TODO: one write, one read */
	usb_send_receive_packet(iqrf->dev);
    	len = usb_read_rx_buff(iqrf->dev, data_buff);

	sem_post(&iqrf->sem);
    	return len;
}



#if 0
int iqrf_write_data(iqrf_t *iqrf, const unsigned char *data_buff, int tx_len)
{
	iqrf_raw_write(iqrf, data_buff, tx_len);
}
#endif

void iqrf_reset_device(iqrf_t *iqrf)
{
	iqrf_lock(iqrf);
	usb_reset(iqrf->dev);
	iqrf_unlock(iqrf);
}

int iqrf_count_tx_crc(unsigned char *buff, int len)
{
    	return count_crc_tx(buff, len);
}


/** Higher level functions **/

/**
 * How to connect a device
 * @param interactive   list devices and let user select one
 * @param list          if interactive == false, list devices only
 * @param which         if (interactive || list) == false, device to connect
 *
 * @return      if list == true || error => NULL, IQRF device otherwise
 */

iqrf_t *iqrf_select_device(bool interactive, bool list, int which)
{
        int n, i;
        int rv;
        usb_addr devs[8];
        iqrf_t *dev;

        n = iqrf_get_device_list(devs, 8);

        if (interactive || list) {
                if (!n) {
                        printf("No devices found\n");
                        return NULL;
                }
                printf("Connected devices:\n");
                for (i=0; i<n; i++) {
                        printf("[%d]\t%02x:%02x\n", i, devs[i].bus, devs[i].port);
                }
        }

        rv = 1;
        if (interactive) {
                printf("which one? ");
                rv = scanf("%d", &which);

        }

        if (rv != 1 || which < 0 || which >= n) {
                printf("canceled [non-existing device]\n");
                return NULL;
        }

        dev = NULL;
        if (!list)
                dev = iqrf_device_open(&devs[which]);

        return dev;
}
