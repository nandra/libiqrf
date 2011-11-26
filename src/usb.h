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

#include <libusb.h>

/* devices identification */
#define CKUSB02_VENDOR_ID_OLD 0x04D8
#define CKUSB02_PRODUCT_ID_OLD 0x000C

#define CKUSB02_VENDOR_ID 0x1DE6
#define CKUSB02_PRODUCT_ID 0x0001

struct iqrf_usb {
    unsigned short vendor_id;
    unsigned short product_id;
};


/* maximum length for rx and tx buffer */
#define BUF_LEN 64

/* 
 *  usb device has only 1 configuration
 * and only 1 interface which consist
 * from 2 interrupt endpoints 
 */

#define OUT_EP_NR (0x01)
#define IN_EP_NR (0x81)

#define USB_TIMEOUT (1000) // in ms

int usb_dev_found();
int init_usb();
int open_usb();
int send_receive_packet();
void reset_usb();
void set_tx_len(int);
void set_rx_len(int);
int read_rx_buff(unsigned char *buff);
void write_tx_buff(unsigned char *buff, int len);
int send_packet();
void release_usb();


#endif // USB_H
