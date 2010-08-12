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
 */


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include "spi.h"
#include "usb.h"
#include <semaphore.h>

#define CMD_FOR_CK 0x01

//#define DEBUG_IQRF_DEV

#ifdef DEBUG_IQRF_DEV
#define DBG(fmt, args...) \
printf(fmt , ##args);
#else
#define DBG(fmt, args...) {}
#endif

static sem_t sem;
   
/* device initialization */
int iqrf_init_device()
{
    	int result = -ENODEV;

    	if (sem_init(&sem, 0 ,1) == -1)
        	perror("sem_init");

    	result = init_usb();

      	if (result)
        	printf("USB error, check device connection\n");
    	
    	return result;
}

/* device release */
void iqrf_release_device(void)
{
	release_usb();
}
	


/* get spi_status of module spi */
int iqrf_get_spi_status(void)
{
    	unsigned char buff[BUF_LEN];
    	int len = 0;
	enum spi_status status;
#ifdef DEBUG_IQRF_DEV
    	time_t tm;
#endif /* DEBUG_IQRF_DEV */
    	sem_wait(&sem);
    	buff[0] = CMD_FOR_CK;
    	buff[1] = 0;
    	buff[2] = 0;
    	buff[3] = 0;
    	buff[4] = 0;
    	set_tx_len(5);
    	set_rx_len(4);

    	write_tx_buff(buff, 5);
    	send_receive_packet();
    	len = read_rx_buff(buff);
    	sem_post(&sem);

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
         	if (buff[1] >= 0x40 && buff[1] <= 0x63) 
              		status = buff[1];
         	else 
		 	status = SPI_UNKNOWN;
         
         	break;
    	}

    	DBG("%lu:spi_status:%X\n", time(&tm), buff[1]);
    	return status;
}

/* get data from spi */
int iqrf_read_write_spi_cmd_data(unsigned char *data_buff, int data_len, int read_write)
{
    	unsigned char buff[BUF_LEN], PTYPE = 0;
    	int i, len, crc_rx, ret_val = 0;

    	sem_wait(&sem);
    	/* avoid get longer data line 35 bytes */
    	data_len &= 0x3F;

    	memset(buff, 0, sizeof(buff));
    	buff[0] = CMD_FOR_CK;
    	buff[1] = SPI_CMD;
    	/* check if master can read write or read only */
    	if (read_write)
        	PTYPE = data_len | 0x80;
    	else
        	PTYPE = data_len & 0x7F;

    	buff[2] = PTYPE;
    	memcpy(&buff[3], data_buff, data_len);
    	buff[data_len + 3] = count_crc_tx(&buff[1], data_len + 3);

    	set_tx_len(data_len + 4);
    	set_rx_len(data_len + 4);
    	for (i = 0; i < data_len + 4; i++)
        	DBG("data transfered[%d]:0x%X\n", i, buff[i]);
    	DBG("\n");
    
	write_tx_buff(buff, data_len + 4);
    	send_receive_packet();
    	len = read_rx_buff(buff);
    	/* count crc for retrieved data */
	crc_rx = check_crc_rx(&buff[2], PTYPE, data_len);

	if (crc_rx) {
		if (read_write) {
			/* whole buff without crc */
			memcpy(data_buff, &buff[0], len - 1);
			DBG("Received data len:0x%x\n", len);
        		for (i = 0; i < len; i++)
            			DBG("%x ", buff[i]);
        		DBG("\n");

        		ret_val = len;

		} else {
        		memcpy(data_buff, &buff[2], data_len);
        		DBG("Received data len:0x%x\n", len);
        		for (i = 2; i < data_len; i++)
            			DBG("%c", buff[i]);
        		DBG("\n");

        		ret_val = data_len;
	}
    	} else {
        	/* this could occur in case of module info */
        	memcpy(data_buff, &buff[2], 4);
        	DBG("Wrong data checksum\n");
    	}
    	sem_post(&sem);
    	return ret_val;
}

/* write and reading data to/from spi */
int iqrf_write_read_data(unsigned char *data_buff, int tx_len, int rx_len, int check_crc)
{
    	unsigned char buff[64], PTYPE;
    	int len, crc_rx, i;

    	sem_wait(&sem);
    	memset(buff, 0, sizeof(buff));
    	memcpy(buff, data_buff, tx_len);
    	for (i=0; i < tx_len; i++)
        	DBG("[%d]=0x%X ", i, buff[i]);
    	DBG("\n");
    	PTYPE = buff[2];
    	set_tx_len(tx_len);
    	set_rx_len(rx_len);

    	write_tx_buff(buff, tx_len);
    	send_receive_packet();
    	len = read_rx_buff(buff);
    	for (i=0; i < len; i++)
        	DBG("rcv[%d]=0x%X ", i, buff[i]);
    	DBG("\n");
    
	if (len && check_crc) {
        	crc_rx = check_crc_rx(&buff[2], PTYPE, len-3);
        	memcpy(data_buff, buff, len);
    	}
    	sem_post(&sem);
    	return len;
}

int iqrf_write_data(unsigned char *data_buff, int tx_len)
{
    	unsigned char buff[64];
    	int i, ret_val = 0;

    	sem_wait(&sem);
    	memset(buff, 0, sizeof(buff));
    	memcpy(buff, data_buff, tx_len);
    	for (i=0; i < tx_len; i++)
        	DBG("[%d]=0x%X ", i, buff[i]);
    	DBG("\n");
    	set_tx_len(tx_len);


    	write_tx_buff(buff, tx_len);
    	ret_val = send_packet();
    	sem_post(&sem);
    	return ret_val;
}

void iqrf_reset_device()
{
    	reset_usb();
}

int iqrf_count_tx_crc(unsigned char *buff, int len)
{
    	return count_crc_tx(buff, len);
}
