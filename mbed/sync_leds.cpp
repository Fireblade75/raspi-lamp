#include "mbed.h"
#include "nRF24L01P.h"

#define MCP_ADDRESS 0x46
#define MCP_NCS D10
#define OPP_GPIOMODE 0x00
#define OPP_GPIOWRITE 0x09
#define OPP_OLAT 0x0A
#define OPP_GPPU 0x06
#define MCP_INTERUPT D6

#define TRANSFER_SIZE   1
#define TRANSFER_ADDRESS 0x581338e

#define NRF24_CSN D8
#define NRF24_CE D9
#define NRF24_IRQ D7

#define _NRF24L01P_REG_SETUP_RETR 0x04

// This code requires a shield with a MCP23S08

SPI MCP(SPI_MOSI, SPI_MISO, SPI_SCK);
nRF24L01P my_nrf24l01p(SPI_MOSI, SPI_MISO, SPI_SCK, NRF24_CSN, NRF24_CE, NRF24_IRQ);
DigitalOut ncs(D10);

int current_led = 0;

void gpioWriteData(char opperation, char data) {
    ncs = 0;
    char data_arr[] = {MCP_ADDRESS, opperation, data};
    char buff[1];
    MCP.write(data_arr, 3, buff, 1);
    ncs = 1;
}

int gpioReadData(char opperation) {
    ncs = 0;
    MCP.write(MCP_ADDRESS ^ 0x1);
    MCP.write(opperation);
    int read_data = MCP.write(0x00);
    //printf("data from %i: %i\r\n", opperation, read_data);
    ncs = 1;
    return read_data;
} 

//#define SETUP_RETR  0x04
//nrf24_configRegister(SETUP_RETR,(0x04<<ARD)|(0x0F<<ARC));
//#define ARD         4 /* 4 bits */
//#define ARC         0 /* 4 bits */
//nrf24_configRegister(0x04,(0x04<<4)|(0x0F<<0));
// (0b0100 << 4) | (0x0F<<0) = 1001111
void nRF24L01P::enableAutoRetransmit(int delay, int count) {
    if(delay < 250 || delay > 4000) {
        printf("Error: delay must be between 250 and 4000 ms");
    } else if(count <= 0 || count >= 16) {
        printf("Error: retransmit count must be between 0 and 15");
    } else {
        int setupTr = ((delay / 250) << 4) + count;
        setRegister(_NRF24L01P_REG_SETUP_RETR, setupTr);
    }
}


int main() {
    // >>> Configure SPI
    MCP.frequency(2000000);
    
    // >>> Configure the nRF24L01
    char txData[TRANSFER_SIZE], rxData[TRANSFER_SIZE];
    int rxDataCnt = 0;

    my_nrf24l01p.powerUp();
    my_nrf24l01p.setRfFrequency(2462);
    my_nrf24l01p.setTxAddress( TRANSFER_ADDRESS );
    my_nrf24l01p.setRxAddress( TRANSFER_ADDRESS );
    my_nrf24l01p.setTransferSize( TRANSFER_SIZE );
    my_nrf24l01p.enableAutoAcknowledge();
    //my_nrf24l01p.disableAutoAcknowledge();
    my_nrf24l01p.enableAutoRetransmit(1000, 5);
    
    printf( "nRF24L01+ Frequency    : %d MHz\r\n",  my_nrf24l01p.getRfFrequency() );
    printf( "nRF24L01+ Output power : %d dBm\r\n",  my_nrf24l01p.getRfOutputPower() );
    printf( "nRF24L01+ Data Rate    : %d kbps\r\n", my_nrf24l01p.getAirDataRate() );
    printf( "nRF24L01+ TX Address   : 0x%010llX\r\n", my_nrf24l01p.getTxAddress() );
    printf( "nRF24L01+ RX Address   : 0x%010llX\r\n", my_nrf24l01p.getRxAddress() );

    my_nrf24l01p.setReceiveMode();
    my_nrf24l01p.enable();
    
    // >>> Configure the GPIO Expander
    ncs = 1;
    wait_ms(50);
    gpioWriteData(OPP_GPIOMODE, 0xC0);
    wait_ms(50);
    gpioWriteData(OPP_GPIOWRITE, 0x3f ^ 1 << current_led);
    gpioWriteData(OPP_GPPU, 0xC0);
    
    while(1) {
        gpioReadData(OPP_OLAT);
        char data = gpioReadData(OPP_GPIOWRITE);
        int btns2 = data & 0x40;
        int btns1 = data & 0x80;
        
        if((btns1 == 0) || (btns2 == 0)) {
            printf("sending button data\r\n");
            if (btns1 == 0) {
                current_led = (current_led + 5) % 6;   
            } else if (btns2 == 0) {
                current_led = (current_led + 1) % 6;
            }
            printf("perform gpio write\r\n");
            gpioWriteData(OPP_GPIOWRITE, 0x3f ^ 1 << current_led);
            
            txData[0] = current_led;
            printf("starting sending process\r\n");
            int write_status = my_nrf24l01p.write( NRF24L01P_PIPE_P0, txData, 1 );
            printf("sending exit code: %i\r\n", write_status);
            
            wait_ms(200);
        }
        
        if ( my_nrf24l01p.readable() ) {
            printf("receiving button data\r\n");
            rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, 1 );
            for ( int i = 0; rxDataCnt > 0; rxDataCnt--, i++ ) {
                printf("reading [%i] => %i\r\n", i, rxData[i]);
                current_led = rxData[i];
                gpioWriteData(OPP_GPIOWRITE, 0x3f ^ 1 << current_led);
            }
        }
    }
}
 