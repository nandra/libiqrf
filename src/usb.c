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

//#define DEBUG_USB

#ifdef DEBUG_USB
#define DBG(fmt, args...) \
printf("libusb:" fmt, ##args);
#else
#define DBG(fmt, args...) {}
#endif


/* table of supported devices */
static struct iqrf_usb devices[] = {
    	{CKUSB02_VENDOR_ID_OLD, CKUSB02_PRODUCT_ID_OLD},
    	{CKUSB02_VENDOR_ID, CKUSB02_PRODUCT_ID},
    	{0, 0},
};

static struct libusb_context *usb_bus;
static struct libusb_device_handle *dev_handle;
static struct libusb_device *dev;
static int found = 0;
static unsigned char tx_buff[BUF_LEN], rx_buff[BUF_LEN];
static unsigned char tx_len, rx_len;

/* observer to get device present status */
int usb_dev_found(void)
{
    	return found;
}

/* usb initialization */
int init_usb()
{
    	found = 0;
	int ret_val = -1;
    	memset(rx_buff, 0, sizeof(rx_buff));
    	memset(tx_buff, 0, sizeof(tx_buff));
	struct libusb_device **list;

    	ssize_t i = 0, cnt;
    	struct libusb_device *device = NULL;

	struct libusb_device_descriptor desc;	 
	
	if ((ret_val = libusb_init(&usb_bus))) {
		perror("usb_init");
		
		goto end;
	}
	cnt = libusb_get_device_list(NULL, &list );
  	
	if (cnt < 0) {
		perror("libusb_get_device_list:");
		ret_val = cnt;
		goto end;
	}
	
	for (i = 0; i < cnt; i++) {
		device = list[i];
		if ((ret_val = libusb_get_device_descriptor(device, &desc))) {
			perror("libusb_get_device_descriptor:");
			goto end;
		}
		
		int j;
   		for (j = 0; j < sizeof(devices); j++) {	
		
			if ((desc.idVendor == devices[j].vendor_id) &&
                		(desc.idProduct == devices[j].product_id)) {

                        	found = 1;
				ret_val = 0;
                        	dev = device;
                        	DBG("USB device found:%x:%x\n", devices[j].vendor_id, devices[j].product_id);
                                /*TODO: for multi-instance handling remove break*/
                        	break;
                 	}
		}
	}	
end:
	if (found) {
		open_usb();
	}
	libusb_free_device_list(list, 1);
	return;	
}

/* opening usb device */
int open_usb()
{
    	int ret_val = 0;

    	if (dev != NULL) {
        	if (!libusb_open(dev, &dev_handle)) {
            		/* claim interface must be done before every
             	 	* write or read to interface
             	 	*/
            		ret_val = libusb_claim_interface(dev_handle, 0);
            		if (ret_val < 0) {
                		perror("usb_claim_interface");
                		ret_val = 0;
            		} else {
                		ret_val = 1;
           		}
        	}
    	}

    	return ret_val;
}

/* receive data from endpoint */
int retrieve_packet()
{
    	int ret_val = 0;
	int transferred;

    	ret_val = libusb_interrupt_transfer(dev_handle, IN_EP_NR,
                                     rx_buff, rx_len,&transferred,
                                     1000);
   	if (ret_val > 0) {
        	rx_len = ret_val;
   	} else {
       		rx_len = 0;
        	perror("usb_irq_read");
    	}

    	return ret_val;
}

/* write data to endpoint */
int send_packet()
{
   	 int ret_val = 0;
	 int transferred;
   	 ret_val = libusb_interrupt_transfer(dev_handle, OUT_EP_NR,
                                      tx_buff, tx_len, &transferred,
                                      1000);
   	 if (ret_val < 0) {
       		 printf("%s\n", __FUNCTION__);
       		 perror("usb_irq_write");
   	 }
   	 return ret_val;
}

/* write and read data to/from endpoint */
int send_receive_packet()
{
   	 int ret_val = 0;

   	 ret_val = send_packet();
   	 if (ret_val)
       		 ret_val = retrieve_packet();

   	 return ret_val;
}

/* set length for transmission */
void set_tx_len(int len)
{
   	 tx_len = len;
}

/* get length of received data */
void set_rx_len(int len)
{
   	 rx_len = len;
}

/* copy rx buffer */
int read_rx_buff(unsigned char * buff)
{
   	 memcpy(buff, rx_buff, rx_len);
   	 return rx_len;
}

/* write to tx buffer */
void write_tx_buff(unsigned char *buff, int len)
{
   	 memcpy(tx_buff, buff, len);
   	 /* clean rx buff */
   	 memset(rx_buff, 0, sizeof(rx_buff));
}

void reset_usb()
{
   	 libusb_reset_device(dev_handle);
}

void release_usb()
{
	if (dev_handle != NULL) {
		libusb_release_interface(dev_handle, 0);
		libusb_close(dev_handle);
		libusb_exit(usb_bus);
    	}
	
	memset(rx_buff, 0, sizeof(rx_buff));
    	memset(tx_buff, 0, sizeof(tx_buff));
    	dev = NULL;
    	dev_handle = NULL;
}
