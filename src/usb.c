/*
 * usb iqrf wrapper
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
#include "usb.h"


#define DEBUG_PREPEND "[debug] usb: "
#include "debug.h"

/* table of supported devices */
static struct iqrf_usb_id devices[] = {
    	{CKUSB02_VENDOR_ID_OLD, CKUSB02_PRODUCT_ID_OLD},
    	{CKUSB02_VENDOR_ID, CKUSB02_PRODUCT_ID},
    	{0, 0},
};

#define USB_MAX_DEVICES 8
struct usb_location used[USB_MAX_DEVICES];


/**
 * usb initialization
 * @return	0 on success, -1 on error
 **/
int init_usb(struct iqrf_usb *self)
{
//    	int found;
	int ret_val;
	struct libusb_device **list;
    	ssize_t i, cnt;

	struct libusb_device *found;
	struct libusb_device_descriptor desc;	 

	memset(self, 0, sizeof(*self));

	ret_val = libusb_init(&self->ctx);
	if (ret_val) {
		perror("usb_init");
		goto err_init;
	}

	cnt = libusb_get_device_list(self->ctx, &list);
  	if (cnt < 0) {
		perror("libusb_get_device_list:");
		ret_val = cnt;
		goto err_list;
	}

	found = NULL;
	for (i = 0; i < cnt; i++) {
	    	struct libusb_device *device;
		device = list[i];

		ret_val = libusb_get_device_descriptor(device, &desc);
		if (ret_val) {
			perror("libusb_get_device_descriptor:");
			goto err_list;
		}

		DBG("device %2d: %04x:%04x\n", i, desc.idVendor, desc.idProduct);
		int j;
   		for (j = 0; j < sizeof(devices); j++) {	
			if ((desc.idVendor == devices[j].vendor_id) &&
                		(desc.idProduct == devices[j].product_id)) {

                        	found = device;
                        	DBG("USB device found:%04x:%04x\n", devices[j].vendor_id, devices[j].product_id);
                                /*TODO: for multi-instance handling remove break*/
                        	break;
                 	}
		}
	}

	/* found? */
	if (!found) {
		DBG("USB device NOT found\n");
		goto err_not_found;
	}

	ret_val = libusb_open(found, &(self->handle));
	if (ret_val) {
		perror("libusb_open");
		goto err_open;
	}

	ret_val = libusb_claim_interface(self->handle, 0);
	if (ret_val) {
		perror("libusb_claim_interface");
		goto err_claim;
	}

	/* free list, but do not unreference it */
	libusb_free_device_list(list, 0);
	DBG("tady\n");
	return 0;

err_claim:
	libusb_close(self->handle);
	self->handle = NULL;
err_open:
	libusb_free_device_list(list, 1);
	list = NULL;
err_not_found:
err_list:
	libusb_exit(self->ctx);
	self->ctx = NULL;
err_init:
	return -1;
}

/* receive data from endpoint */
int retrieve_packet(struct iqrf_usb *self)
{
	int ret_val = 0;
	int transferred;

    	ret_val = libusb_interrupt_transfer(self->handle, IN_EP_NR,
                                     self->rx_buff, self->rx_len, &transferred,
                                     USB_TIMEOUT);
	if (!ret_val)
		ret_val = transferred;
	else
		perror("usb_irq_read");

    	return ret_val;
}

/* write data to endpoint */
int send_packet(struct iqrf_usb *self)
{
	int ret_val = 0;
	int transferred;
	ret_val = libusb_interrupt_transfer(self->handle, OUT_EP_NR,
					self->tx_buff, self->tx_len, &transferred,
					USB_TIMEOUT);

	if (ret_val < 0) {
       		 perror("usb_irq_write");
	}

	return ret_val;
}

/* write and read data to/from endpoint */
int send_receive_packet(struct iqrf_usb *self)
{
	int ret_val = 0;

	DBG("<send_receive_packet> %p\n", self);
	ret_val = send_packet(self);
	if (!ret_val)
		ret_val = retrieve_packet(self);
	DBG("</send_receive_packet>\n");
	return ret_val;
}

/* set length for transmission */
void set_tx_len(struct iqrf_usb *self, int len)
{
   	 self->tx_len = len;
}

/* get length of received data */
void set_rx_len(struct iqrf_usb *self, int len)
{
   	 self->rx_len = len;
}

/* copy rx buffer */
int read_rx_buff(struct iqrf_usb *self, unsigned char * buff)
{
   	 memcpy(buff, self->rx_buff, self->rx_len);
   	 return self->rx_len;
}

/* write to tx buffer */
void write_tx_buff(struct iqrf_usb *self, unsigned char *buff, int len)
{
   	 memcpy(self->tx_buff, buff, len);
   	 /* clean rx buff */
   	 memset(self->rx_buff, 0, sizeof(self->rx_buff));
}

void reset_usb(struct iqrf_usb *self)
{
   	 libusb_reset_device(self->handle);
}

void release_usb(struct iqrf_usb *self)
{
	if (self->handle != NULL) {
		libusb_release_interface(self->handle, 0);
		libusb_close(self->handle);
		libusb_exit(NULL);
    	}
	
	memset(self->rx_buff, 0, sizeof(self->rx_buff));
    	memset(self->tx_buff, 0, sizeof(self->tx_buff));
//    	dev = NULL;
    	self->handle = NULL;
}
