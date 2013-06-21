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


#ifndef USB_H
#define USB_H

#include <stdint.h>	// requires C99
#include <stdbool.h>
#include <libusb.h>

/* devices identification */
#define CKUSB02_VENDOR_ID_OLD 0x04D8
#define CKUSB02_PRODUCT_ID_OLD 0x000C

#define CKUSB02_VENDOR_ID 0x1DE6
#define CKUSB02_PRODUCT_ID 0x0001

typedef struct {
    unsigned short vendor_id;
    unsigned short product_id;
} usb_id;

typedef struct {
	uint8_t bus;
	uint8_t port;
} usb_addr;

/* maximum length for rx and tx buffer */
#define BUF_LEN 64

typedef struct {
	/* USB device */
	struct libusb_device_handle *handle;
	/* IQRF device */
	unsigned char tx_buff[BUF_LEN], rx_buff[BUF_LEN];
	unsigned char tx_len, rx_len;
} device_t;


int usb_init();
void usb_exit();
int usb_get_device_list(usb_addr devices[], int n_devices, bool count_only);

device_t *usb_device_open_by_addr(usb_addr *addr);
void usb_device_close(device_t *dev);

int usb_retrieve_packet(device_t *self);
int usb_send_packet(device_t *self);
int usb_send_receive_packet(device_t *self);
void usb_set_tx_len(device_t *self, int len);
void usb_set_rx_len(device_t *self, int len);
int usb_read_rx_buff(device_t *self, unsigned char * buff);
void usb_write_tx_buff(device_t *self, const unsigned char *buff, int len);
void usb_reset(device_t *self);

void usb_tx_buff_write(device_t *dev, const unsigned char *buff, int len);

/* 
 *  usb device has only 1 configuration
 * and only 1 interface which consist
 * from 2 interrupt endpoints 
 */

#define OUT_EP_NR (0x01)
#define IN_EP_NR (0x81)

#define USB_TIMEOUT (1000) // in ms

#endif // USB_H
