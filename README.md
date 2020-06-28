# Raspi Lamp

This is a tiny module to connect an I2C RGB lamp to a Pi and make it possible 
for other devices to configure it by using Bluetooth Low Energy.

To run this project you need NodeJS and the Raspberry Pi bluetooth software.\
You can install the bluetooth software with the following command on Raspbain/Debian/Ubuntu\
`sudo apt install bluetooth bluez libbluetooth-dev libudev-dev`

## Libraries
- raspi-i2c
- noble
