#ifndef SPI_H
#define SPI_H

#define SPI_DATA_LENGTH        (35)
#define SPI_CRC_DEFAULT        (0x5F)

#define SPI_CMD 0xF0
/* posiible spi statuses */
#define NO_MODULE_ON_USB        0xFF    // SPI not working (HW error)
#define SPI_DISABLED            0x00    // SPI not working (disabled)
#define SPI_CRCM_OK             0x3F    // SPI not ready (full buffer, last CRCM ok)
#define SPI_CRCM_ERR            0x3E    // SPI not ready (full buffer, last CRCM error)
#define COMMUNICATION_MODE      0x80    // SPI ready (communication mode)
#define PROGRAMMING_MODE        0x81    // SPI ready (programming mode)
#define DEBUG_MODE              0x82    // SPI ready (debugging mode)
#define SPI_SLOW_MODE           0x83    // SPI is not working on the background - Slow mode
#define SPI_USER_STOP           0x07    // state after stopSPI();
#define SPI_DATA_READY          0x40    // data ready

class iqrf_spi {
public:
    iqrf_spi();
    ~iqrf_spi();
    unsigned char count_crc_tx(unsigned char *, int);
    unsigned char check_crc_rx(unsigned char *, int , int );

};

#endif // SPI_H
