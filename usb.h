#ifndef USB_H
#define USB_H

#include <usb.h>


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

/* usb device has only 1 configuration
 * and only 1 interface which consist
 * from 2 interrupt endpoints */
#define OUT_EP_NR (0x01)
#define IN_EP_NR (0x81)



class lusb{
    public:
        int status;
        lusb();
        ~lusb();
        int usb_dev_found();
        void init_usb();
        int open_usb();
        int send_receive_packet();
        void reset_usb();
        void set_tx_len(int);
        void set_rx_len(int);
        int read_rx_buff(unsigned char *buff);
        void write_tx_buff(unsigned char *buff, int len);
        int send_packet();
        void release_usb();
    private:

        int retrieve_packet();
        struct usb_bus *usb_bus;
        struct usb_dev_handle *dev_handle;
        struct usb_device *dev;
        int found;
        char tx_buff[BUF_LEN], rx_buff[BUF_LEN];
        char tx_len, rx_len;
};

#endif // USB_H
