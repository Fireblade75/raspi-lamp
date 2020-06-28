/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
#include "ble/BLE.h"
#include "platform/mbed_thread.h"
#include "pretty_printer.h"



// Blinking rate in milliseconds
#define BLINKING_RATE_MS  500
#define CHARACTERISTIC_LENGTH 20
#define DEVICE_NAME "NRF_52WC"

#define UUIDdevice "29f47dbc-fbb4-477f-95e8-8a3452d3249a"  
#define UUIDlightintensity "346c99c9-f69f-48df-90d0-40d28de06407"
#define UUIDservice "d9298dad-5480-4c92-a494-28a6cc02b786"                                                 

Serial pc(USBTX, USBRX);

//Create and set the Devicename characteristic, so the Pi knows that it is the NRF
static uint8_t deviceNameCharacteristicValue[CHARACTERISTIC_LENGTH] = DEVICE_NAME;
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(deviceNameCharacteristicValue)> \
    deviceNameCharacteristic(
    UUIDdevice,
    deviceNameCharacteristicValue);

//Create and set the lightIntensity, basevalue 50. It must be initializer list or string literal
static uint8_t lightIntensityCharacteristicValue[CHARACTERISTIC_LENGTH] =  {50};
ReadOnlyArrayGattCharacteristic<uint8_t, sizeof(lightIntensityCharacteristicValue)> \
    lightIntensityCharacteristic(
    UUIDlightintensity,
    lightIntensityCharacteristicValue);


/**
 * Callback triggered when the ble initialization process has finished
 *
 * @param[in] params Information about the initialized Peripheral
 */
void onBluetoothInitialized( BLE::InitializationCompleteCallbackContext *params)
{
    if (params->error != BLE_ERROR_NONE) {
            print_error(params->error, "Ble initialization failed:" );
            return;
        } else {
            print_error(params->error, "Ble initialization success" );    
        }

        //print_mac_address();

        //start_advertising();   
}

/**
 * Callback handler when a Central has disconnected
 *
 * @param[i] params Information about the connection
 */
void onCentralDisconnected( const Gap::DisconnectionCallbackParams_t *params);


int main()
{
    pc.printf("starting \n \r");
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
    
    //create and initialize BLE
    BLE &ble = BLE::Instance();
    ble.init(this, onBluetoothInitialized);
    
    while(ble.hasInitialized() == false)
    {
        wait(1);
        pc.printf("Not yet initialized \n \r");
    }
    GattCharacteristic *characteristics[] = {&deviceNameCharacteristic, &lightIntensityCharacteristic};
    
    //create the service, add the characteristics and add those to the gattServer
    GattService lightService(UUIDservice, characteristics, sizeof(characteristics) / sizeof(GattCharacteristic *));
    ble.gattServer().addService(lightService);
    
    //set devicename on advertisements
    ble.gap().setDeviceName((uint8_t const *)DEVICE_NAME);

    //Setup advertising payload
    ble.gap().accumulateAdvertisingPayload(
        GapAdvertisingData::BREDR_NOT_SUPPORTED |
        GapAdvertisingData::LE_GENERAL_DISCOVERABLE);
    
    ble.gap().accumulateAdvertisingPayload(
        GapAdvertisingData::SHORTENED_LOCAL_NAME,
        (uint8_t const *)DEVICE_NAME, sizeof(DEVICE_NAME) - 1);
    
    //set advertising type, interval and then start advertising
    ble.gap().setAdvertisingType(
        GapAdvertisingParams::
        ADV_CONNECTABLE_UNDIRECTED);

    ble.gap().setAdvertisingInterval(250);
    ble.gap().startAdvertising();
    
    
    while (true) {
        led = !led;
        thread_sleep_for(BLINKING_RATE_MS);
    }
}