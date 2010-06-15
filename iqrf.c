#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include "iqrf.h"

//#define DEBUG_IQRF_DEV

#ifdef DEBUG_IQRF_DEV
#define DBG(fmt, args...) \
printf(fmt , ##args);
#else
#define DBG(fmt, args...) {}
#endif

/* constructor */
iqrf_dev::iqrf_dev()
{
    this->usb = new lusb();
    this->spi = new iqrf_spi();
    if (sem_init(&this->sem, 0 ,1) == -1)
        perror("sem_init");
}

/* destructor */
iqrf_dev::~iqrf_dev()
{
    delete(this->usb);
    delete(this->spi);
}

int iqrf_dev::init_device()
{
    int result = 0;
    this->usb->init_usb();
    if (this->usb->usb_dev_found()) {
        result = this->usb->open_usb();
        if (!result)
            printf("USB error, check device connection\n");
    }
    return result;
}

/* get spi_status of module spi */
int iqrf_dev::get_spi_status(void)
{
    unsigned char buff[64];
    int len = 0;
#ifdef DEBUG_IQRF_DEV
    time_t tm;
#endif /* DEBUG_IQRF_DEV */
    sem_wait(&this->sem);
    buff[0] = CMD_FOR_CK;
    buff[1] = 0;
    buff[2] = 0;
    buff[3] = 0;
    buff[4] = 0;
    this->usb->set_tx_len(5);
    this->usb->set_rx_len(4);

    this->usb->write_tx_buff(buff, 5);
    this->usb->send_receive_packet();
    len = this->usb->read_rx_buff(buff);
    sem_post(&this->sem);

    switch(buff[1]) {
    case SPI_DISABLED:
         this->spi_status = SPI_DISABLED;
         break;
    case SPI_USER_STOP:
         this->spi_status = SPI_USER_STOP;
         break;
    case SPI_CRCM_OK:
         this->spi_status = SPI_CRCM_OK;
         break;
    case SPI_CRCM_ERR:
         this->spi_status = SPI_CRCM_ERR;
         break;
    case COMMUNICATION_MODE:
         this->spi_status = COMMUNICATION_MODE;
         break;
    case PROGRAMMING_MODE:
         this->spi_status = PROGRAMMING_MODE;
         break;
    case DEBUG_MODE:
         this->spi_status = DEBUG_MODE;
         break;
    case SPI_SLOW_MODE:
         this->spi_status = SPI_SLOW_MODE;
         break;
    case NO_MODULE_ON_USB:
         this->spi_status = NO_MODULE_ON_USB;
         break;
    default:
         if (buff[1] >= 0x40 && buff[1] <= 0x63) {
              this->spi_status = SPI_DATA_READY;
         } else {
            fprintf(stderr,"Unkown SPI response!\n");
         }
         break;
    }

    DBG("%lu:spi_status:%X\n", time(&tm), buff[1]);
    return buff[1];
}

/* get data from spi */
int iqrf_dev::get_spi_cmd_data(unsigned char *data_buff, int data_len, int read_write)
{
    unsigned char buff[64], PTYPE = 0;
    int i, len, crc_rx, ret_val = 0;

    sem_wait(&this->sem);
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
    buff[data_len + 3] = this->spi->count_crc_tx(&buff[1], data_len + 3);

    this->usb->set_tx_len(data_len + 4);
    this->usb->set_rx_len(data_len + 4);
    for (i = 0; i < data_len + 4; i++)
        DBG("data transfered[%d]:0x%X", i, buff[i]);
    DBG("\n");
    this->usb->write_tx_buff(buff, data_len + 4);
    this->usb->send_receive_packet();
    len = this->usb->read_rx_buff(buff);
    /* count crc for retrieved data */
    crc_rx = this->spi->check_crc_rx(&buff[2], PTYPE, data_len);

    if (crc_rx) {
        memcpy(data_buff, &buff[2], data_len);
        DBG("Received data len:0x%x\n", len);
        for (i = 2; i < data_len; i++)
            DBG("%c", buff[i]);
        DBG("\n");

        ret_val = data_len;
    } else {
        /* this could occur in case of module info */
        memcpy(data_buff, &buff[2], 4);
        DBG("Wrong data checksum\n");
    }
    sem_post(&this->sem);
    return ret_val;
}

/* write and reading data to/from spi */
int iqrf_dev::write_read_data(unsigned char *data_buff, int tx_len, int rx_len, int check_crc)
{
    unsigned char buff[64], PTYPE;
    int len, crc_rx, i;

    sem_wait(&this->sem);
    memset(buff, 0, sizeof(buff));
    memcpy(buff, data_buff, tx_len);
    for (i=0; i < tx_len; i++)
        DBG("[%d]=0x%X ", i, buff[i]);
    DBG("\n");
    PTYPE = buff[2];
    this->usb->set_tx_len(tx_len);
    this->usb->set_rx_len(rx_len);

    this->usb->write_tx_buff(buff, tx_len);
    this->usb->send_receive_packet();
    len = this->usb->read_rx_buff(buff);
    for (i=0; i < len; i++)
        DBG("rcv[%d]=0x%X ", i, buff[i]);
    DBG("\n");
    if (len && check_crc) {
        crc_rx = this->spi->check_crc_rx(&buff[2], PTYPE, len-3);
        memcpy(data_buff, buff, len);
    }
    sem_post(&this->sem);
    return len;
}

int iqrf_dev::write_data(unsigned char *data_buff, int tx_len)
{
    unsigned char buff[64];
    int i, ret_val = 0;

    sem_wait(&this->sem);
    memset(buff, 0, sizeof(buff));
    memcpy(buff, data_buff, tx_len);
    for (i=0; i < tx_len; i++)
        DBG("[%d]=0x%X ", i, buff[i]);
    DBG("\n");
    this->usb->set_tx_len(tx_len);


    this->usb->write_tx_buff(buff, tx_len);
    ret_val = this->usb->send_packet();
    sem_post(&this->sem);
    return ret_val;
}

void iqrf_dev::reset_device()
{
    this->usb->reset_usb();
}

int iqrf_dev::count_crc(unsigned char *buff, int len)
{
    return this->spi->count_crc_tx(buff, len);
}
