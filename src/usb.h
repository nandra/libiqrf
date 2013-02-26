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
#include <libusb.h>

/* devices identification */
#define CKUSB02_VENDOR_ID_OLD 0x04D8
#define CKUSB02_PRODUCT_ID_OLD 0x000C

#define CKUSB02_VENDOR_ID 0x1DE6
#define CKUSB02_PRODUCT_ID 0x0001

struct iqrf_usb_id {
    unsigned short vendor_id;
    unsigned short product_id;
};

struct usb_location {
	uint8_t bus;
	uint8_t addr;
};


/* maximum length for rx and tx buffer */
#define BUF_LEN 64

struct iqrf_usb {
	struct libusb_context *ctx;
	struct libusb_device_handle *handle;
	unsigned char tx_buff[BUF_LEN], rx_buff[BUF_LEN];
	unsigned char tx_len, rx_len;
};
/* 
 *  usb device has only 1 configuration
 * and only 1 interface which consist
 * from 2 interrupt endpoints 
 */

#define OUT_EP_NR (0x01)
#define IN_EP_NR (0x81)

#define USB_TIMEOUT (1000) // in ms

//int usb_dev_found();
int init_usb(struct iqrf_usb *);
int open_usb(struct iqrf_usb *);
int send_receive_packet(struct iqrf_usb *);
void reset_usb(struct iqrf_usb *);
void set_tx_len(struct iqrf_usb *, int);
void set_rx_len(struct iqrf_usb *, int);
int read_rx_buff(struct iqrf_usb *, unsigned char *buff);
void write_tx_buff(struct iqrf_usb *, unsigned char *buff, int len);
int send_packet(struct iqrf_usb *);
void release_usb(struct iqrf_usb *);


#endif // USB_H
