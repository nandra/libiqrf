#include <stdio.h>
#include <string.h>
#include "lusb.h"

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

/* constructor */
lusb::lusb()
{
    memset(this->rx_buff, 0, sizeof(this->rx_buff));
    memset(this->tx_buff, 0, sizeof(this->tx_buff));
    this->dev = NULL;
    this->dev_handle = NULL;
}

/* destructor */
lusb::~lusb()
{
    if (this->dev_handle != NULL) {
        usb_release_interface(this->dev_handle, 0);
        usb_close(this->dev_handle);
    }
}

/* observer to get device preset status */
int lusb::usb_dev_found()
{
    return found;
}

/* usb initialization */
void lusb::init_usb()
{
    found = 0;
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
                                    this->found = 1;
                                    this->dev = device;
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
int lusb::open_usb()
{
    int ret_val = 0;

    if (this->dev != NULL) {
        this->dev_handle = usb_open(this->dev);
        if (this->dev_handle != NULL) {
            /* claim interface must be done before every
             * write or read to interface
             */
            ret_val = usb_claim_interface(this->dev_handle, 0);
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
int lusb::retrieve_packet()
{
    int ret_val = 0;

    ret_val = usb_interrupt_read(this->dev_handle, IN_EP_NR,
                                     this->rx_buff, this->rx_len,
                                     1000);
    if (ret_val > 0) {
        this->rx_len = ret_val;
    } else {
        this->rx_len = 0;
        perror("usb_irq_read");
    }

    return ret_val;
}

/* write data to endpoint */
int lusb::send_packet()
{
    int ret_val = 0;

    ret_val = usb_interrupt_write(this->dev_handle, OUT_EP_NR,
                                      this->tx_buff, this->tx_len,
                                      1000);
    if (ret_val < 0) {
        printf("%s\n", __FUNCTION__);
        perror("usb_irq_write");
    }
    return ret_val;
}

/* write and read data to/from endpoint */
int lusb::send_receive_packet()
{
    int ret_val = 0;

    ret_val = this->send_packet();
    if (ret_val)
        ret_val = this->retrieve_packet();

    return ret_val;

}

/* set length for transmission */
void lusb::set_tx_len(int len = 0)
{
    this->tx_len = len;
}

/* get length of received data */
void lusb::set_rx_len(int len = 0)
{
    this->rx_len = len;
}

/* copy rx buffer */
int lusb::read_rx_buff(unsigned char * buff)
{
    memcpy(buff, this->rx_buff, this->rx_len);
    return this->rx_len;
}

/* write to tx buffer */
void lusb::write_tx_buff(unsigned char *buff, int len)
{
    memcpy(this->tx_buff, buff, len);
    /* clean rx buff */
    memset(this->rx_buff, 0, sizeof(this->rx_buff));
}

void lusb::reset_usb()
{
    usb_reset(this->dev_handle);
}

void lusb::release_usb()
{
    memset(this->rx_buff, 0, sizeof(this->rx_buff));
    memset(this->tx_buff, 0, sizeof(this->tx_buff));
    this->dev = NULL;
    this->dev_handle = NULL;
}
