// Include the required Wire library for I2C
#include <Wire.h>
#define RED_LED 7
#define GREEN_LED 8
#define BLUE_LED 9

#define BRIGHTNESS_REGISTER 10
#define RED_REGISTER 1
#define GREEN_REGISTER 2
#define BLUE_REGISTER 3

int LED = 13;

int registerAddress = -1;
int registerValue = 0
int transferDone = 0;

int brightness = 128;
bool useRed = true;
bool useGreen = false;
bool useBlue = false;

void setup() {
    pinMode (RED_LED, OUTPUT);
    pinMode (GREEN_LED, OUTPUT);
    pinMode (BLUE_LED, OUTPUT);
    
    Wire.begin(9); // Start the I2C Bus as Slave on address 9
    Wire.onReceive(receiveEvent);
}

void receiveEvent(int bytes) {
    if(registerAddress == -1) {
        registerAddress = Wire.read();
        if(registerAddress < 0) {
            registerAddress = -1;
        }
    } else {
        registerValue = Wire.read();
        transferDone = 1;
    }
}

void loop() {
    //If value received is 0 blink LED for 200 ms
    if (registerAddress >= 0) {
        // brightness 
        if(registerAddress == BRIGHTNESS_REGISTER) {
            if(registerValue >= 0 && registerValue <= 128) {
                brightness = registerValue; 
            }
        } else if(registerAddress == RED_REGISTER) {
            useRed = registerValue;
        } else if(registerAddress == GREEN_REGISTER) {
            useGreen = registerValue;
        } else if(registerAddress == BLUE_REGISTER) {
            useBlue = registerValue;
        }

        if(useRed) {
            analogWrite(RED_LED, brightness * 2); 
        } else {
            analogWrite(RED_LED, 0); 
        }
        if(useGreen) {
            analogWrite(GREEN_LED, brightness * 2); 
        } else {
            analogWrite(GREEN_LED, 0); 
        }
        if(useBlue) {
            analogWrite(BLUE_LED, brightness * 2); 
        } else {
            analogWrite(BLUE_LED, 0); 
        }
        
        registerAddress = -1;
        transferDone = 0;
    }
}