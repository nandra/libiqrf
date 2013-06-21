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
#include <stdbool.h>
#include "usb.h"


#define DEBUG_PREPEND "[debug] usb: "
#include "debug.h"

/* table of supported devices */
static usb_id devices[] = {
    	{CKUSB02_VENDOR_ID_OLD, CKUSB02_PRODUCT_ID_OLD},
    	{CKUSB02_VENDOR_ID, CKUSB02_PRODUCT_ID},
    	{0, 0},
};
/* context */
struct libusb_context *usb_ctx = NULL;


static bool device_supported(struct libusb_device *dev)
{
	struct libusb_device_descriptor desc;
	int rv;
	int i;

	rv =  libusb_get_device_descriptor(dev, &desc);
	if (rv)
		return false;
	i=0;
	while (devices[i].vendor_id) {
		if (devices[i].vendor_id == desc.idVendor
				&& devices[i].product_id == desc.idProduct)
			return true;
		i++;
	}
	return false;
}

int usb_init(void)
{
	if (usb_ctx)
		ERR("usb_init(): again?\n");
	return libusb_init(&usb_ctx);
}

void usb_exit(void)
{
	libusb_exit(usb_ctx);
	usb_ctx = NULL;
}

static void usb_device_get_addr(struct libusb_device *dev, usb_addr *addr)
{
	addr->bus = libusb_get_bus_number(dev);
	addr->port = libusb_get_port_number(dev);
}
/**
 * TODO:
 */
bool try_open(struct libusb_device *dev)
{
	(void)dev;
	return true;
}

int usb_get_device_list(usb_addr devices[], int n_devices, bool count_only)
{
	int i, j;
	int cnt;
	struct libusb_device **list;

	/* clear list? */
	if (count_only == false)
		memset(devices, 0, n_devices * sizeof(usb_addr));

	cnt = libusb_get_device_list(usb_ctx, &list);
	if (cnt < 0)
		return cnt;

	j = 0;
	for (i=0; i < cnt && j < n_devices; i++) {
		if (device_supported(list[i]) && try_open(list[i])) {
			/* add to list? */
			if (count_only == false) {
				usb_device_get_addr(list[i], &devices[j]);
			}
			j++;
		}
	}
	libusb_free_device_list(list, 1);

	return j;
}

device_t *usb_device_open(struct libusb_device *usbdev)
{
	device_t *dev;
	int rv;

	dev = malloc(sizeof(*dev));
	if (!dev)
		goto err_malloc;

	memset(dev, 0, sizeof(*dev));
	rv = libusb_open(usbdev, &(dev->handle));
	if (rv) {
		perror("libusb_open");
		goto err_open;
	}

	rv = libusb_claim_interface(dev->handle, 0);
	if (rv)
		goto err_claim;

	return dev;
	/* error handling */
err_claim:
	libusb_close(dev->handle);
err_open:
	free(dev);
err_malloc:
	DBG("usb_device_open() failed\n");
	return NULL;
}

void usb_device_close(device_t *dev)
{
	libusb_release_interface(dev->handle, 0);
	libusb_close(dev->handle);
	free(dev);
}

device_t *usb_device_open_by_addr(usb_addr *addr)
{
	int i;
	int cnt;
	struct libusb_device **list;
	device_t *device;
	usb_addr curr;

	cnt = libusb_get_device_list(usb_ctx, &list);
	if (cnt < 0)
		return NULL;

	for (i=0; i<cnt; i++) {
		usb_device_get_addr(list[i], &curr);
		if (!memcmp(addr, &curr, sizeof(curr)))
			break;
	}

	device = NULL;
	if (i == cnt)
		goto end;

	if (!device_supported(list[i]))
		goto end;

	device = usb_device_open(list[i]);
end:
	libusb_free_device_list(list, 1);
	return device;
}


/**
 * @param direction	0 - incoming data, 1 - outgoing data
 */

static void debug_print_packet(const unsigned char *data, int len, int direction)
{
	(void)data;
	(void)len;
	(void)direction;
#if 0
	int i;
	DBG("Dumping %s data:\n", direction?"outgoing":"incoming");
	for (i=0; i < len; i++)
        	DBG("%s[%d]=0x%X ", direction?"out":"in", i, data[i]);
    	DBG("\n");
#endif

}

/* receive data from endpoint */
int usb_retrieve_packet(device_t *dev)
{
	int ret_val = 0;
	int transferred;

    	ret_val = libusb_interrupt_transfer(dev->handle, IN_EP_NR,
                                     dev->rx_buff, dev->rx_len, &transferred,
                                     USB_TIMEOUT);

	if (ret_val) {
		/* TODO: print libusb error */
		return -1;
	}

	debug_print_packet(dev->rx_buff, dev->rx_len, 0);
	return transferred;	
}

/* write data to endpoint */
int usb_send_packet(device_t *dev)
{
	int ret_val = 0;
	int transferred;
	ret_val = libusb_interrupt_transfer(dev->handle, OUT_EP_NR,
					dev->tx_buff, dev->tx_len, &transferred,
					USB_TIMEOUT);

	if (ret_val) {
		/* TODO: print libusb error */
       		 perror("usb_irq_write");
		 return -1;
	}

	debug_print_packet(dev->tx_buff, dev->tx_len, 1);
	return transferred;
//	return ret_val;
}

/* write and read data to/from endpoint */
int usb_send_receive_packet(device_t *dev)
{
	int ret_val = 0;

	ret_val = usb_send_packet(dev);
	if (ret_val < 0)
		return -1;

//	if (!ret_val)
	ret_val = usb_retrieve_packet(dev);
	return ret_val;
}


/**
 * Only setters & getters
 */


/* set length for transmission */
void usb_set_tx_len(device_t *dev, int len)
{
   	 dev->tx_len = len;
}

/* get length of received data */
void usb_set_rx_len(device_t *dev, int len)
{
   	 dev->rx_len = len;
}

/* copy rx buffer */
int usb_read_rx_buff(device_t *dev, unsigned char * buff)
{
   	 memcpy(buff, dev->rx_buff, dev->rx_len);
   	 return dev->rx_len;
}


void usb_tx_buff_clear(device_t *dev)
{
	memset(dev->tx_buff, 0, sizeof(dev->tx_buff));
}

void usb_tx_buff_write(device_t *dev, const unsigned char *buff, int len)
{
	usb_tx_buff_clear(dev);
	memcpy(dev->tx_buff, buff, len);
	/* clean rx buff */
	memset(dev->rx_buff, 0, sizeof(dev->rx_buff));
}

/* write to tx buffer */
void usb_write_tx_buff(device_t *dev, const unsigned char *buff, int len)
{
	usb_tx_buff_write(dev, buff, len);
}





void usb_reset(device_t *dev)
{
   	 libusb_reset_device(dev->handle);
}

