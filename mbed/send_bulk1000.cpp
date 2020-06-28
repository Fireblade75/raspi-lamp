#include "mbed.h"
#include "nRF24L01P.h"

#define MCP_ADDRESS 0x46
#define MCP_NCS D10
#define OPP_GPIOMODE 0x00
#define OPP_GPIOWRITE 0x09
#define OPP_OLAT 0x0A
#define OPP_GPPU 0x06
#define MCP_INTERUPT D6

#define TRANSFER_SIZE   4
#define TRANSFER_ADDRESS 0x581338e

#define NRF24_CSN D8
#define NRF24_CE D9
#define NRF24_IRQ D7
#define MESSAGE_COUNT 10000

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

//void clearWriteBuffer() {
//    for (int i = 0; i < TRANSFER_SIZE; i++) {
//        txData[i] = ' ';
//    }
//}

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

void enbladeLED(char led_nr) {
    gpioWriteData(OPP_GPIOWRITE, 0x3f ^ 1 << led_nr);    
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
    
        // >>> Configuration for the benchmarks
    // RF_FREQUENCY min: 2400, max: 2525
    my_nrf24l01p.setRfFrequency(2462);
    
    //  NRF24L01P_TX_PWR_ZERO_DBm NRF24L01P_TX_PWR_MINUS_6_DB, NRF24L01P_TX_PWR_MINUS_12_DB, NRF24L01P_TX_PWR_MINUS_18_DB
    my_nrf24l01p.setRfOutputPower(NRF24L01P_TX_PWR_MINUS_12_DB);
    
    // NRF24L01P_DATARATE_250_KBPS, NRF24L01P_DATARATE_1_MBPS, NRF24L01P_DATARATE_2_MBPS
    my_nrf24l01p.setAirDataRate(NRF24L01P_DATARATE_250_KBPS);
    
    my_nrf24l01p.disableAutoAcknowledge();
    my_nrf24l01p.disableAutoRetransmit();
    
    wait_ms(200);
    
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
    
    enbladeLED(1);
    
    // >>> Set up Timers
    Timer senderTimer;
    Timer receiverTimer;
    int messageCount = 0;
    
    while(1) {
        gpioReadData(OPP_OLAT);
        char data = gpioReadData(OPP_GPIOWRITE);
        int btns2 = (data & 0x40);
        int btns1 = (data & 0x80);
        
        if((btns1 == 0) || (btns2 == 0)) {
            senderTimer.start();
            for(int current_int = 1; current_int < MESSAGE_COUNT; current_int++) {        
                txData[0] = current_int >> 24;
                txData[1] = current_int >> 16 & 0xFF;
                txData[2] = current_int >> 8 & 0xFF;
                txData[3] = current_int & 0xFF;
                    
                int write_status = my_nrf24l01p.write( NRF24L01P_PIPE_P0, txData, TRANSFER_SIZE);
                if(write_status != TRANSFER_SIZE) {
                    printf("Failed to write data, %i bytes written\r\n", write_status);    
                }
            }
            senderTimer.stop();
            printf("It took %i ms to send all messages\n", senderTimer.read_ms());
        }
        
        if ( my_nrf24l01p.readable() ) {
            rxDataCnt = my_nrf24l01p.read( NRF24L01P_PIPE_P0, rxData, TRANSFER_SIZE );
            if(rxDataCnt == 4) {
                int rxInt = (rxData[0] << 24) + (rxData[1] << 16) + (rxData[2] << 8) + rxData[3];
                messageCount++;
                if(rxInt == 1) {
                    enbladeLED(2);    
                    receiverTimer.start();
                    messageCount = 1;
                } else if(rxInt == MESSAGE_COUNT) {
                    enbladeLED(4);  
                    receiverTimer.stop();
                    printf("%i messages received\r\n", messageCount);
                    printf("It took %i ms to receive all messages\r\n", receiverTimer.read_ms());
                }
            }
        }
    }
}
 