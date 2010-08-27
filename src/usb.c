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
printf("lusb:" fmt, ##args);
#else
#define DBG(fmt, args...) {}
#endif


/* table of supported devices */
static struct iqrf_usb devices[] = {
    	{CKUSB02_VENDOR_ID_OLD, CKUSB02_PRODUCT_ID_OLD},
    	{CKUSB02_VENDOR_ID, CKUSB02_PRODUCT_ID},
    	{0, 0},
};

static struct usb_bus *usb_bus;
static struct usb_dev_handle *dev_handle;
static struct usb_device *dev;
static int found;

#define IRQ_ENDPOINT (3)
#define BULK_ENDPOINT (2)

static int usb_transfer_type;

static unsigned char tx_buff[BUF_LEN], rx_buff[BUF_LEN];
static unsigned char tx_len, rx_len;

/* observer to get device present status */
int usb_dev_found(void)
{
    	return found;
}

/* usb initialization */
void init_usb()
{
    	found = 0;
    	memset(rx_buff, 0, sizeof(rx_buff));
    	memset(tx_buff, 0, sizeof(tx_buff));
    	dev = NULL;
    	dev_handle = NULL;

    	unsigned int i = 0;
    	usb_bus = usb_get_busses();
    	struct usb_device *device;
   
       	usb_init();
    	usb_find_busses();
    	usb_find_devices();
    	for (usb_bus = usb_busses; usb_bus; usb_bus = usb_bus->next) {
        	for (device = usb_bus->devices; device; device = device->next) {
                        for (i = 0; i < sizeof(devices); i++) {
                        	if ((device->descriptor.idVendor == devices[i].vendor_id) &&
                               		(device->descriptor.idProduct == devices[i].product_id)) {

                                	if (device != NULL) {
                                    		found = 1;
                                    		dev = device;
						/* newer devices use bulk transfer instead of interrupt */
						if ((device->config->interface->altsetting->endpoint->bmAttributes & IRQ_ENDPOINT) == IRQ_ENDPOINT) {
							usb_transfer_type = IRQ_ENDPOINT;
						} else if ((device->config->interface->altsetting->endpoint->bmAttributes & IRQ_ENDPOINT) == BULK_ENDPOINT) {
							usb_transfer_type = BULK_ENDPOINT;
						} else {
							fprintf(stderr, "Unknown endpoint transfer type!!!\n");
							found = 0;
							break;
						}


						DBG("attr:%x\n",device->config->interface->altsetting->endpoint->bmAttributes);
                                    		DBG("USB device found:%x:%x\n", devices[i].vendor_id, devices[i].product_id);
                                    		/*TODO: for multi-instance handling remove break*/
                                    		break;
                                	}

                            	}
                        }

                }
        }

}

/* opening usb device */
int open_usb()
{
    	int ret_val = 0;

    	if (dev != NULL) {
        	dev_handle = usb_open(dev);
        	if (dev_handle != NULL) {
            	/* claim interface must be done before every
             	 * write or read to interface
             	 */
            		ret_val = usb_claim_interface(dev_handle, 0);
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

    	if (usb_transfer_type == BULK_ENDPOINT) 
		ret_val = usb_bulk_read(dev_handle, IN_EP_NR,
                                     rx_buff, rx_len,
                                     1000);
	else
		ret_val = usb_bulk_read(dev_handle, IN_EP_NR,
                                     rx_buff, rx_len,
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

   	 if (usb_transfer_type == BULK_ENDPOINT)
	 	ret_val = usb_bulk_write(dev_handle, OUT_EP_NR,
                                      tx_buff, tx_len,
                                      1000);
	 else
	 	ret_val = usb_bulk_write(dev_handle, OUT_EP_NR,
                                      tx_buff, tx_len,
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
   	 usb_reset(dev_handle);
}

void release_usb()
{
	if (dev_handle != NULL) {
		usb_release_interface(dev_handle, 0);
		usb_close(dev_handle);
    	}
	
	memset(rx_buff, 0, sizeof(rx_buff));
    	memset(tx_buff, 0, sizeof(tx_buff));
    	dev = NULL;
    	dev_handle = NULL;
	usb_transfer_type = 0;
}
