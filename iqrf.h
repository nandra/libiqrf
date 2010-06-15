#ifndef IQRF_DEV_H
#define IQRF_DEV_H

#include "lusb.h"
#include "spi.h"
#include <semaphore.h>

#define CMD_FOR_CK 0x01

class iqrf_dev {
  public:
    iqrf_dev();
    ~iqrf_dev();
    iqrf_spi *spi;
    lusb *usb;
    int get_spi_status(void);
    int get_spi_cmd_data(unsigned char *data_buff, int data_len, int read_write);
    int write_read_data(unsigned char *data_buff, int tx_len, int rx_len, int check_crc);
    int write_data(unsigned char *data_buff, int tx_len);
    int spi_status;
    sem_t sem;
    int init_device();
    void reset_device();
    int count_crc(unsigned char *buff, int len);
  private:

};

#endif // IQRF_DEV_H
